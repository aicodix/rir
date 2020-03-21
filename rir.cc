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
struct Compute
{
	static const int kernel_length = filter_length / 2;
	typedef typename cmplx::value_type value;
	DSP::RealToHalfComplexTransform<filter_length, cmplx> fwd;
	DSP::FastFourierTransform<kernel_length, cmplx, 1> bwd;
	value input[filter_length];
	cmplx kernel[kernel_length];
	cmplx tmp[kernel_length];
	cmplx rir[kernel_length];
	cmplx avg[kernel_length];
	Compute(DSP::WriteWAV<value> &output_file, DSP::ReadWAV<value> &input_file, DSP::ReadWAV<value> &filter_file, int clip_length)
	{
		filter_file.read(input, filter_length);
		fwd(kernel, input);
		for (int i = 0; i < kernel_length; ++i)
			kernel[i] = conj(kernel[i]);
		for (int i = 0; i < kernel_length; ++i)
			kernel[i] /= value(filter_length) * sqrt(kernel_length);
		input_file.skip(filter_length / 2);
		input_file.read(input, filter_length);
		while (input_file.good()) {
			fwd(tmp, input);
			for (int i = 0; i < kernel_length; ++i)
				tmp[i] *= kernel[i];
			bwd(rir, tmp);
			for (int i = 0; i < kernel_length; ++i)
				avg[i] += rir[i];
			input_file.read(input, filter_length);
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

	if (filter_file.channels() != 1 || input_file.channels() != 1) {
		std::cerr << "Only real signal (one channel) supported." << std::endl;
		return 1;
	}

	if (filter_file.rate() != input_file.rate()) {
		std::cerr << "Samplerates of input and filter files don't match." << std::endl;
		return 1;
	}

	DSP::WriteWAV<value> output_file(output_name, input_file.rate(), input_file.bits(), 2);

	switch (filter_file.frames()) {
	case 16384:
		new Compute<cmplx, 16384>(output_file, input_file, filter_file, clip_length);
		break;
	case 65536:
		new Compute<cmplx, 65536>(output_file, input_file, filter_file, clip_length);
		break;
	case 262144:
		new Compute<cmplx, 262144>(output_file, input_file, filter_file, clip_length);
		break;
	case 1048576:
		new Compute<cmplx, 1048576>(output_file, input_file, filter_file, clip_length);
		break;
	default:
		std::cerr << "Unsupported filter length." << std::endl;
		return 1;
	}
	return 0;
}

