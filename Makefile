
CXXFLAGS = -std=c++11 -W -Wall -Ofast -fno-exceptions -fno-rtti -march=native -I../dsp
CXX = clang++ -stdlib=libc++
#CXX = g++

.PHONY: all

all: rir noise chirp paths

rir: rir.cc
	$(CXX) $(CXXFLAGS) $< -o $@

noise: noise.cc
	$(CXX) $(CXXFLAGS) $< -o $@

chirp: chirp.cc
	$(CXX) $(CXXFLAGS) $< -o $@

paths: paths.cc
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean

clean:
	rm -f rir noise chirp paths

