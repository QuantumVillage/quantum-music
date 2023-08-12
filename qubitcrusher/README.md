# Qubit Crusher

This is the repo for QubitCrusher - a distortion effect based on the (in)famous `bitcrusher` distortion effect, the source for which can be found here: [bitcrusher demo download](https://www.korg.com/us/support/download/others/0/832/4743/).

We have taken this effect and made it _QUANTUM_! You can download it ready to use RIGHT NOW on a KORG Nu:tekt NTS-1 here: [qubitcrusher.ntkdigunit](./qubitcrusher.ntkdigunit)

## Wait, 'made it quantum' how?

The bitcrusher effect relies on the device 'skipping' a given number of samples per 64 sample block. The more samples, the more distorted it sounds. 

We first setup the variables we will need, including the statevector which is a reference to the statevector's main components - `cos` and `sin`. These are loaded as functin pointers in the `state_vctr` array:

```c
float (*state_vctr[4])(float) {fastercosf, fastersinf, fastersinf, fastercosf};
int vector_values[4] = {0}; uint8_t distribution[32] = {0};
```

Whenever there is a change in the time/delay dial, we are thrown into interrupts, so when something is changed we are sent into `MODFX_PARAM`, where we process the value returned as the input for our Rx gate:

```c
for(int i = 0; i < 4; i++){
    float theta = (valf * M_PI)/2;
    float p = state_vctr[i]((int)theta);
    float v = M_1_SQRT2 * p;
    float v2 = v * v;
    vector_values[i] = (int)(v2 * 32);
};
```

We then need to setup a distribution from the output of the statevector, which is done with:

```c
int place = 0;
for(int i = 4; i > 0; i--){
    int t = vector_values[i];
    for(int j = 0; j < t; j++){
        distribution[place] = i;
        place++;
    }
}
```

When `theta == 0` we have `[00,00,00,00,00,00,00,00,00,00,11,11,11,11,11,11,11,11,11,11]`, and when `theta == 1` it will have become `[01,01,01,01,01,01,01,01,01,01,10,10,10,10,10,10,10,10,10,10]`. 

With this, we can then just choose a random value which is mathematically equivalent to simulating the circuit:

```c
uint32_t r1 = fx_rand() % 32; uint32_t r2 = fx_rand() % 32;
uint32_t quantum_shift = distribution[r1] << distribution[r2];
uint32_t skip = (rate * 32) + quantum_shift;
```

And that's how you make bitcrusher to QUbitcrusher! 