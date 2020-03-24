
Compute Room [Impulse Response](https://en.wikipedia.org/wiki/Impulse_response) by [circular cross-correlation](https://en.wikipedia.org/wiki/Discrete_Fourier_transform#Circular_convolution_theorem_and_cross-correlation_theorem) of a repeating noise source and its recording.

### Quick start

Generate 48 KHz, 16 bit, 65536 samples long noise sequences with 30 different seeds (each sequence repeated two times):

```
./noise noise.wav filter.wav 48000 16 65536 30
```

Start recording and interrupt after noise stops:

```
arecord -r 48000 -c1 -f S16_LE -V mono -s $((65536 * 2 * (30 + 5))) input.wav
```

Play sequences:

```
aplay noise.wav
```

Compute Room Impulse Response and output only 4097 (use -1 for all) samples, centered at the peak:

```
./rir output.wav input.wav filter.wav 4097
```

### Using a chirp signal instead of noise

Generate a 8 KHz, 16 bit, 65536 samples long chirp signal, repeated 30 times and going from 500 Hz to 3500 Hz exponentially (use 1 for linear chirp):

```
./chirp chirps.wav filter.wav 8000 16 65536 30 500 3500 0
```

Play signal:

```
aplay chirps.wav
```

Start recording and interrupt before chirping stops:

```
arecord -r 48000 -c1 -f S16_LE -V mono -s $((65536 * (30 - 5))) input.wav
```

Compute Room Impulse Response and output only 4097 (use -1 for all) samples, centered at the peak:

```
./rir output.wav input.wav filter.wav 4097
```

### [rir.cc](rir.cc)

Compute Room [Impulse Response](https://en.wikipedia.org/wiki/Impulse_response).

### [noise.cc](noise.cc)

[Pseudorandom noise](https://en.wikipedia.org/wiki/Pseudorandom_noise) generator for multiple sequences.

### [chirp.cc](chirp.cc)

[Chirp](https://en.wikipedia.org/wiki/Chirp) generator with repeating of signal.

### [paths.cc](paths.cc)

Show strongest paths

