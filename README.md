
Compute Room [Impulse Response](https://en.wikipedia.org/wiki/Impulse_response) by [circular cross-correlation](https://en.wikipedia.org/wiki/Discrete_Fourier_transform#Circular_convolution_theorem_and_cross-correlation_theorem) of a repeating noise source and its recording.

### Quick start

Generate a 48 KHz, 16 bit, 65536 samples long noise sequence, repeated 30 times:

```
./noise noise.wav 48000 16 65536 30
```

Play sequence:

```
aplay noise.wav
```

Start recording and interrupt before noise stops:

```
arecord -r 48000 -c1 -f S16_LE -V mono -s $((65536 * (30 - 5))) input.wav
```

Compute Room Impulse Response:

```
./rir output.wav input.wav noise.wav 65536
```

### [rir.cc](rir.cc)

Compute Room [Impulse Response](https://en.wikipedia.org/wiki/Impulse_response).

### [noise.cc](noise.cc)

[Pseudorandom noise](https://en.wikipedia.org/wiki/Pseudorandom_noise) generator with sequence repeating.

