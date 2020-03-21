/*
Show strongest paths

Copyright 2020 Ahmet Inan <inan@aicodix.de>
*/

#include <iostream>
#include <cmath>
#include "complex.hh"
#include "wav.hh"

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "usage: " << argv[0] << " INPUT" << std::endl;
		return 1;
	}

	const char *input_name = argv[1];

	typedef float value;
	typedef DSP::Complex<value> cmplx;

	DSP::ReadWAV<value> input_file(input_name);

	if (input_file.channels() != 2) {
		std::cerr << "Only analytic signal (two channels) supported." << std::endl;
		return 1;
	}

	int length = input_file.frames();
	cmplx *input = new cmplx[length];
	input_file.read(reinterpret_cast<value *>(input), length, 2);

	if (0) {
		for (int i = 0; i < length; ++i)
			std::cout << i << " " << input[i].real() << " " << input[i].imag() << std::endl;
		return 0;
	}

	const int max_paths = 30;
	int paths[max_paths] = { 0 };
	for (int k = 0; k < length; ++k) {
		if (abs(input[paths[max_paths-1]]) < abs(input[k])) {
			for (int j = 0; j < max_paths; ++j) {
				if (abs(input[paths[j]]) < abs(input[k])) {
					for (int i = max_paths-1; i > j; --i)
						paths[i] = paths[i-1];
					paths[j] = k;
					break;
				}
			}
		}
	}
	for (int i = 0; i < max_paths; ++i) {
		value ampl = abs(input[paths[i]]);
		int delay = (paths[i] - paths[0] + length) % length;
		value msec = 1000 * value(delay) / input_file.rate();
		value rad = arg(input[paths[i]]);
		std::cout << ampl << " " << msec << " " << rad << std::endl;
	}

	return 0;
}

