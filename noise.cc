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
	if (argc != 6) {
		std::cerr << "usage: " << argv[0] << " OUTPUT RATE BITS LENGTH REPEAT" << std::endl;
		return 1;
	}

	const char *name = argv[1];
	int rate = std::atoi(argv[2]);
	int bits = std::atoi(argv[3]);
	int length = std::atoi(argv[4]);
	int repeat = std::atoi(argv[5]);

	typedef float value;

	std::random_device rd;
	typedef std::default_random_engine generator;
	typedef std::bernoulli_distribution bernoulli;
	auto noise = std::bind(bernoulli(), generator(rd()));
	value *input = new value[length];
	for (int i = 0; i < length; ++i)
		input[i] = 1 - 2 * noise();

	DSP::WriteWAV<value> file(name, rate, bits, 1);
	for (int i = 0; i < repeat; ++i)
		file.write(input, length);

	return 0;
}

