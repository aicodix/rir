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
	Compute(DSP::WriteWAV<value> &output_file, DSP::ReadWAV<value> &input_file, DSP::ReadWAV<value> &noise_file)
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
		output_file.write(reinterpret_cast<value *>(avg+max_pos), kernel_length-max_pos);
		output_file.write(reinterpret_cast<value *>(avg), max_pos);
		//for (int i = 0; i < kernel_length; ++i)
		//	std::cout << i << " " << avg[i].real() << " " << avg[i].imag() << std::endl;
	}
};

int main(int argc, char **argv)
{
	if (argc != 5) {
		std::cerr << "usage: " << argv[0] << " OUTPUT INPUT NOISE LENGTH" << std::endl;
		return 1;
	}

	const char *output_name = argv[1];
	const char *input_name = argv[2];
	const char *noise_name = argv[3];
	int noise_length = std::atoi(argv[4]);

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
		new Compute<cmplx, 16384>(output_file, input_file, noise_file);
		break;
	case 65536:
		new Compute<cmplx, 65536>(output_file, input_file, noise_file);
		break;
	case 262144:
		new Compute<cmplx, 262144>(output_file, input_file, noise_file);
		break;
	case 1048576:
		new Compute<cmplx, 1048576>(output_file, input_file, noise_file);
		break;
	default:
		std::cerr << "Unsupported noise length." << std::endl;
		return 1;
	}
	return 0;
}

