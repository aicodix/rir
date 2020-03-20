/*
Chirp generator

Copyright 2020 Ahmet Inan <inan@aicodix.de>
*/

#include <iostream>
#include <cmath>
#include "complex.hh"
#include "const.hh"
#include "wav.hh"

int main(int argc, char **argv)
{
	if (argc != 10) {
		std::cerr << "usage: " << argv[0] << " OUTPUT FILTER RATE BITS LENGTH REPEAT FREQ0 FREQ1 LINEAR" << std::endl;
		return 1;
	}

	const char *output_name = argv[1];
	const char *filter_name = argv[2];
	int rate = std::atoi(argv[3]);
	int bits = std::atoi(argv[4]);
	int length = std::atoi(argv[5]);
	int repeat = std::atoi(argv[6]);
	int freq0 = std::atof(argv[7]);
	int freq1 = std::atof(argv[8]);
	int linear = std::atoi(argv[9]);

	typedef float value;
	value f0 = freq0 / value(rate);
	value f1 = freq1 / value(rate);
	value T = length;
	value c = (f1 - f0) / T;
	value k = pow(f1 / f0, 1 / T);
	value *output = new value[length];

	for (int i = 0; i < length; ++i)
		if (linear)
			output[i] = sin(DSP::Const<value>::TwoPi() * ((((c/2)*i)*i) + f0*i));
		else
			output[i] = sin(DSP::Const<value>::TwoPi() * f0 * (pow(k,i)-1) / log(k));

	DSP::WriteWAV<value> output_file(output_name, rate, bits, 1);
	for (int i = 0; i < repeat; ++i)
		output_file.write(output, length);

	if (!linear)
		for (int i = 0; i < length; ++i)
			output[i] /= pow(k,i);

	DSP::WriteWAV<value> filter_file(filter_name, rate, bits, 1);
	filter_file.write(output, length);

	return 0;
}

