#include "pico/stdlib.h"
#include <complex.h>
int measure(char circuit[6][2][8], double complex stateVec[4]);
void processSlice(char slice[4][8]);
uint8_t qmidi(uint8_t val);
void q_instrument(uint8_t RXval, int len, uint8_t results[len], uint8_t resb[len*4]);

