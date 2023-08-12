#include <complex.h>
#include <math.h>
#include <stdlib.h>
#include "pico/stdlib.h"
int simulate(double complex stateVec[4], uint8_t results[4]);
int simulate_n(double complex stateVec[4], uint8_t results[], int len);