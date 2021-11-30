/*
Room impulse response

Copyright 2020 Ahmet Inan <inan@aicodix.de>
*/

#include <iostream>
#include <cmath>
#include "complex.hh"
#include "fft.hh"
#include "wav.hh"

template <typename cmplx, int filter_length>
struct MultipleSequences
{
	static const int kernel_length = filter_length;
	const int filter_count;
	typedef typename cmplx::value_type value;
	DSP::FastFourierTransform<filter_length, cmplx, -1> fwd;
	DSP::FastFourierTransform<kernel_length, cmplx, 1> bwd;
	cmplx input[filter_length];
	cmplx fdom[kernel_length];
	cmplx tmp[kernel_length];
	cmplx avg[kernel_length];
	MultipleSequences(DSP::WriteWAV<value> &output_file, DSP::ReadWAV<value> &input_file, DSP::ReadWAV<value> &filter_file, int clip_length) : filter_count(filter_file.channels())
	{
		value *frame = new value[filter_count];
		cmplx *filter = new cmplx[filter_length*filter_count];
		for (int j = 0; j < filter_length; ++j) {
			filter_file.read(frame, 1);
			for (int i = 0; i < filter_count; ++i)
				filter[filter_length*i+j] = frame[i];
		}
		cmplx *kernel = new cmplx[kernel_length*filter_count];
		for (int i = 0; i < filter_count; ++i)
			fwd(kernel+kernel_length*i, filter+filter_length*i);
		delete[] filter;
		for (int i = 0; i < kernel_length*filter_count; ++i)
			kernel[i] = conj(kernel[i]);
		for (int i = 0; i < kernel_length*filter_count; ++i)
			kernel[i] /= value(filter_length) * sqrt(kernel_length);
		cmplx *rir = new cmplx[kernel_length*filter_count];
		cmplx *val_max = new cmplx[filter_count];
		value *abs_max = new value[filter_count];
		int *pos_max = new int[filter_count];
		bool *filter_used = new bool[filter_count];
		for (int i = 0; i < filter_count; ++i)
			filter_used[i] = false;
		int last = -1;
		int count = 0;
		while (input_file.good()) {
			for (int i = 0; i < filter_length/2; ++i)
				input[i] = input[i+filter_length/2];
			input_file.read(reinterpret_cast<value *>(input+filter_length/2), filter_length/2, 2);
			fwd(fdom, input);
			for (int j = 0; j < filter_count; ++j) {
				for (int i = 0; i < kernel_length; ++i)
					tmp[i] = fdom[i] * kernel[kernel_length*j+i];
				bwd(rir+kernel_length*j, tmp);
				val_max[j] = 0;
				abs_max[j] = 0;
				pos_max[j] = 0;
				for (int i = 0; i < kernel_length; ++i) {
					if (abs_max[j] < abs(rir[kernel_length*j+i])) {
						val_max[j] = rir[kernel_length*j+i];
						abs_max[j] = abs(rir[kernel_length*j+i]);
						pos_max[j] = i;
					}
				}
			}
			int max0 = 0, max1 = 0;
			for (int i = 1; i < filter_count; ++i) {
				if (abs_max[max0] < abs_max[i]) {
					max1 = max0;
					max0 = i;
				} else if (abs_max[max1] < abs_max[i]) {
					max1 = i;
				}
			}
			int chosen = max0;
			if (filter_used[chosen]) {
				chosen = last = -1;
			} else if (max0 != max1 && abs_max[max0] < 10 * abs_max[max1]) {
				chosen = -1;
			} else if (last != chosen) {
				last = chosen;
				chosen = -1;
			} else {
				filter_used[chosen] = true;
				if (clip_length < 0)
					clip_length = kernel_length;
				int shift = (clip_length-1)/2;
				int offset = (pos_max[chosen]-shift+kernel_length)%kernel_length;
				int rest = kernel_length-offset;
				int okay = std::min(clip_length, rest);
				for (int i = 0; i < okay; ++i)
					avg[i] += rir[kernel_length*chosen+offset+i] / val_max[chosen];
				for (int i = 0; i < clip_length-rest; ++i)
					avg[okay+i] += rir[kernel_length*chosen+i] / val_max[chosen];
				++count;
			}
		}
		for (int i = 0; i < clip_length; ++i)
			avg[i] /= count;
		output_file.write(reinterpret_cast<value *>(avg), clip_length, 2);
		delete[] filter_used;
		delete[] val_max;
		delete[] abs_max;
		delete[] pos_max;
		delete[] frame;
		delete[] kernel;
		delete[] rir;
	}
};

template <typename cmplx, int filter_length>
struct RepeatedSequence
{
	static const int kernel_length = filter_length;
	typedef typename cmplx::value_type value;
	DSP::FastFourierTransform<filter_length, cmplx, -1> fwd;
	DSP::FastFourierTransform<kernel_length, cmplx, 1> bwd;
	cmplx input[filter_length];
	cmplx kernel[kernel_length];
	cmplx tmp[kernel_length];
	cmplx rir[kernel_length];
	cmplx avg[kernel_length];
	RepeatedSequence(DSP::WriteWAV<value> &output_file, DSP::ReadWAV<value> &input_file, DSP::ReadWAV<value> &filter_file, int clip_length)
	{
		filter_file.read(reinterpret_cast<value *>(input), filter_length, 2);
		fwd(kernel, input);
		for (int i = 0; i < kernel_length; ++i)
			kernel[i] = conj(kernel[i]);
		for (int i = 0; i < kernel_length; ++i)
			kernel[i] /= value(filter_length) * sqrt(kernel_length);
		input_file.skip(filter_length / 2);
		input_file.read(reinterpret_cast<value *>(input), filter_length, 2);
		while (input_file.good()) {
			fwd(tmp, input);
			for (int i = 0; i < kernel_length; ++i)
				tmp[i] *= kernel[i];
			bwd(rir, tmp);
			for (int i = 0; i < kernel_length; ++i)
				avg[i] += rir[i];
			input_file.read(reinterpret_cast<value *>(input), filter_length, 2);
		}
		cmplx val_max;
		value abs_max = 0;
		int max_pos = 0;
		for (int i = 0; i < kernel_length; ++i) {
			if (abs_max < abs(avg[i])) {
				val_max = avg[i];
				abs_max = abs(avg[i]);
				max_pos = i;
			}
		}
		for (int i = 0; i < kernel_length; ++i)
			avg[i] /= val_max;

		if (clip_length < 0)
			clip_length = kernel_length;
		int shift = (clip_length-1)/2;
		int offset = (max_pos-shift+kernel_length)%kernel_length;
		int rest = kernel_length-offset;
		output_file.write(reinterpret_cast<value *>(avg+offset), std::min(clip_length, rest), 2);
		output_file.write(reinterpret_cast<value *>(avg), std::max(0, clip_length-rest), 2);
	}
};

int main(int argc, char **argv)
{
	if (argc != 5) {
		std::cerr << "usage: " << argv[0] << " OUTPUT INPUT FILTER CLIP" << std::endl;
		return 1;
	}

	const char *output_name = argv[1];
	const char *input_name = argv[2];
	const char *filter_name = argv[3];
	int clip_length = std::atoi(argv[4]);

	typedef float value;
	typedef DSP::Complex<value> cmplx;

	DSP::ReadWAV<value> filter_file(filter_name);
	DSP::ReadWAV<value> input_file(input_name);

	if (input_file.channels() != 1) {
		std::cerr << "Only real signal (one channel) supported." << std::endl;
		return 1;
	}

	if (filter_file.rate() != input_file.rate()) {
		std::cerr << "Samplerates of input and filter files don't match." << std::endl;
		return 1;
	}

	DSP::WriteWAV<value> output_file(output_name, input_file.rate(), input_file.bits(), 1);

	bool multiple = filter_file.channels() > 1;

	switch (filter_file.frames()) {
	case 16384:
		if (multiple)
			new MultipleSequences<cmplx, 16384>(output_file, input_file, filter_file, clip_length);
		else
			new RepeatedSequence<cmplx, 16384>(output_file, input_file, filter_file, clip_length);
		break;
	case 65536:
		if (multiple)
			new MultipleSequences<cmplx, 65536>(output_file, input_file, filter_file, clip_length);
		else
			new RepeatedSequence<cmplx, 65536>(output_file, input_file, filter_file, clip_length);
		break;
	case 262144:
		if (multiple)
			new MultipleSequences<cmplx, 262144>(output_file, input_file, filter_file, clip_length);
		else
			new RepeatedSequence<cmplx, 262144>(output_file, input_file, filter_file, clip_length);
		break;
	case 1048576:
		if (multiple)
			new MultipleSequences<cmplx, 1048576>(output_file, input_file, filter_file, clip_length);
		else
			new RepeatedSequence<cmplx, 1048576>(output_file, input_file, filter_file, clip_length);
		break;
	default:
		std::cerr << "Unsupported filter length." << std::endl;
		return 1;
	}
	return 0;
}

