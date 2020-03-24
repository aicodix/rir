/*
Noise generator

Copyright 2020 Ahmet Inan <inan@aicodix.de>
*/

#include <iostream>
#include <cmath>
#include <random>
#include <functional>
#include "complex.hh"
#include "wav.hh"

int main(int argc, char **argv)
{
	if (argc != 7) {
		std::cerr << "usage: " << argv[0] << " OUTPUT FILTER RATE BITS LENGTH COUNT" << std::endl;
		return 1;
	}

	const char *output_name = argv[1];
	const char *filter_name = argv[2];
	int rate = std::atoi(argv[3]);
	int bits = std::atoi(argv[4]);
	int length = std::atoi(argv[5]);
	int count = std::atoi(argv[6]);

	typedef float value;

	std::random_device rd;
	typedef std::default_random_engine generator;
	typedef std::bernoulli_distribution bernoulli;
	auto noise = std::bind(bernoulli(), generator(rd()));
	value *filter = new value[length*count];
	for (int j = 0; j < count; ++j)
		for (int i = 0; i < length; ++i)
			filter[count*i+j] = 1 - 2 * noise();
	DSP::WriteWAV<value> filter_file(filter_name, rate, bits, count);
	filter_file.write(filter, length);

	value *output = new value[length];
	DSP::WriteWAV<value> output_file(output_name, rate, bits, 1);
	for (int j = 0; j < count; ++j) {
		for (int i = 0; i < length; ++i)
			output[i] = filter[count*i+j];
		for (int i = 0; i < 2; ++i)
			output_file.write(output, length);
	}

	return 0;
}

