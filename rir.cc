/*
Room impulse response

Copyright 2020 Ahmet Inan <inan@aicodix.de>
*/

#include <iostream>
#include <cmath>
#include <random>
#include <functional>
#include "complex.hh"
#include "utils.hh"
#include "fft.hh"
#include "wav.hh"

template <typename cmplx, int noise_length>
struct Compute
{
	static const int kernel_length = noise_length / 2;
	typedef typename cmplx::value_type value;
	DSP::RealToHalfComplexTransform<noise_length, cmplx> fwd;
	DSP::FastFourierTransform<kernel_length, cmplx, 1> bwd;
	value input[noise_length];
	cmplx kernel[kernel_length];
	cmplx tmp[kernel_length];
	cmplx rir[kernel_length];
	cmplx avg[kernel_length];
	Compute(DSP::WriteWAV<value> &output_file, DSP::ReadWAV<value> &input_file, DSP::ReadWAV<value> &noise_file, int clip_length)
	{
		noise_file.read(input, noise_length);
		fwd(kernel, input);
		for (int i = 0; i < kernel_length; ++i)
			kernel[i] = conj(kernel[i]);
		for (int i = 0; i < kernel_length; ++i)
			kernel[i] /= value(noise_length) * sqrt(kernel_length);
		input_file.skip(noise_length / 2);
		input_file.read(input, noise_length);
		while (input_file.good()) {
			fwd(tmp, input);
			for (int i = 0; i < kernel_length; ++i)
				tmp[i] *= kernel[i];
			bwd(rir, tmp);
			for (int i = 0; i < kernel_length; ++i)
				avg[i] += rir[i];
			input_file.read(input, noise_length);
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

		const int max_paths = 30;
		int paths[max_paths] = { 0 };
		for (int k = 0; k < kernel_length; ++k) {
			if (abs(avg[paths[max_paths-1]]) < abs(avg[k])) {
				for (int j = 0; j < max_paths; ++j) {
					if (abs(avg[paths[j]]) < abs(avg[k])) {
						for (int i = max_paths-1; i > j; --i)
							paths[i] = paths[i-1];
						paths[j] = k;
						break;
					}
				}
			}
		}
		for (int i = 0; i < max_paths; ++i) {
			value ampl = abs(avg[paths[i]]);
			int delay = (paths[i] - paths[0] + kernel_length) % kernel_length;
			value msec = 1000 * value(delay) / input_file.rate();
			value rad = arg(avg[paths[i]]);
			std::cout << ampl << " " << msec << " " << rad << std::endl;
		}

		//for (int i = 0; i < kernel_length; ++i)
		//	std::cout << i << " " << avg[i].real() << " " << avg[i].imag() << std::endl;
	}
};

int main(int argc, char **argv)
{
	if (argc != 6) {
		std::cerr << "usage: " << argv[0] << " OUTPUT INPUT NOISE LENGTH CLIP" << std::endl;
		return 1;
	}

	const char *output_name = argv[1];
	const char *input_name = argv[2];
	const char *noise_name = argv[3];
	int noise_length = std::atoi(argv[4]);
	int clip_length = std::atoi(argv[5]);

	typedef float value;
	typedef DSP::Complex<value> cmplx;

	DSP::ReadWAV<value> noise_file(noise_name);
	DSP::ReadWAV<value> input_file(input_name);

	if (noise_file.channels() != 1 || input_file.channels() != 1) {
		std::cerr << "Only real signal (one channel) supported." << std::endl;
		return 1;
	}

	if (noise_file.rate() != input_file.rate()) {
		std::cerr << "Samplerate of input and noise files don't match." << std::endl;
		return 1;
	}

	DSP::WriteWAV<value> output_file(output_name, input_file.rate(), input_file.bits(), 2);

	switch (noise_length) {
	case 16384:
		new Compute<cmplx, 16384>(output_file, input_file, noise_file, clip_length);
		break;
	case 65536:
		new Compute<cmplx, 65536>(output_file, input_file, noise_file, clip_length);
		break;
	case 262144:
		new Compute<cmplx, 262144>(output_file, input_file, noise_file, clip_length);
		break;
	case 1048576:
		new Compute<cmplx, 1048576>(output_file, input_file, noise_file, clip_length);
		break;
	default:
		std::cerr << "Unsupported noise length." << std::endl;
		return 1;
	}
	return 0;
}

