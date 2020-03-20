
Compute Room [Impulse Response](https://en.wikipedia.org/wiki/Impulse_response) by [circular cross-correlation](https://en.wikipedia.org/wiki/Discrete_Fourier_transform#Circular_convolution_theorem_and_cross-correlation_theorem) of a repeating noise source and its recording.

### Quick start

Generate a 48 KHz, 16 bit, 65536 samples long noise sequence, repeated 30 times:

```
./noise noise.wav filter.wav 48000 16 65536 30
```

Play sequence:

```
aplay noise.wav
```

Start recording and interrupt before noise stops:

```
arecord -r 48000 -c1 -f S16_LE -V mono -s $((65536 * (30 - 5))) input.wav
```

Compute Room Impulse Response and output only (use -1 for all) 4097 samples centered at the peak:

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

Start recording and interrupt before noise stops:

```
arecord -r 48000 -c1 -f S16_LE -V mono -s $((65536 * (30 - 5))) input.wav
```

Compute Room Impulse Response and output only (use -1 for all) 4097 samples centered at the peak:

```
./rir output.wav input.wav filter.wav 4097
```

### [rir.cc](rir.cc)

Compute Room [Impulse Response](https://en.wikipedia.org/wiki/Impulse_response).

### [noise.cc](noise.cc)

[Pseudorandom noise](https://en.wikipedia.org/wiki/Pseudorandom_noise) generator with sequence repeating.

### [chirp.cc](chirp.cc)

[Chirp](https://en.wikipedia.org/wiki/Chirp) generator with repeating of signal.

