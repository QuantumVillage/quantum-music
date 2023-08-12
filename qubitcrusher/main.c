#include "usermodfx.h"
#include "float_math.h"
#include "fx_api.h" // for the RNG function fx_rand()

static float rate, lastSampleL, lastSampleR;
static int32_t count;

// the random distribution from the statevector
uint8_t distribution[32] = {0};

/*
 * The circuit in use is the following:
 * --[H]--------c--
 *              |
 * -[Rx(theta)]-X--
 * 
 * The starting state is always |00>.
 * 
 * This lets us use the control wheel (DEPTH control B) to rotate around the X axis,
 * with the TIME (control A) wheel used to control the mix. 
 * 
 * Rotating the DEPTH control will switch between an entanglement of 1/sqrt(2) (|00>+|11>)
 * and 1/sqrt(2) (|01>+|10>), with a balanced superposition of 
 * 1/sqrt(4) (|00>+|01>+|10>+|11>) at a half-way point. This is quite
 * fast to compute, and lets us mimic the other QMusic distortion units released by
 * Quantum Village.
*/

// the statevector from our circuit
// formed here from functions in the SDK from float_math.h
// NB - the actual vector is all 'times 1/sqrt(2)', or '* M_1_SQRT2', 
// which is done later. 
float (*state_vctr[4])(float) = {fastercosf, fastersinf, fastersinf, fastercosf};
// a place to store the value outputs
int vector_values[4] = {0};

void MODFX_INIT(uint32_t platform, uint32_t api)
{
  lastSampleL = 0.f;
  lastSampleR = 0.f;
  count = 0;
}

void MODFX_PROCESS(const float *main_xn, float *main_yn,
                   const float *sub_xn, float *sub_yn,
                   uint32_t frames)
{
  //大きくなるほどサンプルが粗くなる
  //get random values - mod32 as this is the length of our distribution array
  uint32_t r1 = fx_rand() % 32;
  uint32_t r2 = fx_rand() % 32;
  // get quantum shift from distribution taken from our balanced fx_rand input...
  uint32_t quantum_shift = distribution[r1] << distribution[r2];
  uint32_t skip = (rate * 32) + quantum_shift;

  for (uint32_t i = 0; i < frames; i++)
  {

    const float inL = main_xn[i*2];
    const float inR = main_xn[i*2+1];

    if (count == 0)
    {
      lastSampleL = inL;
      lastSampleR = inR;
    }

    // update the output samples
    main_yn[i*2] = lastSampleL;
    main_yn[i*2+1] = lastSampleR;
    count++;

    //skip samples if count == skip is reached
    if (count > (int)skip)
      count = 0;
  }
}

void MODFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index)
  {
  case k_user_modfx_param_time:
    rate = valf;
    break;
  case k_user_modfx_param_depth:
    for(int i = 0; i < 4; i++){
      // pass the input to the vector sin/cosine functions
      float theta = (valf * M_PI)/2;
      float p = state_vctr[i]((int)theta);
      // whole vector is 'times 1/sqrt(2)' so do that here
      float v = M_1_SQRT2 * p;
      float v2 = v * v;
      // times 32 as this is the length of our array...
      vector_values[i] = (int)(v2 * 32);
    };
    int place = 0;
    for(int i = 4; i > 0; i--){
      int t = vector_values[i];
      for(int j = 0; j < t; j++){
        distribution[place] = i;
        place++;
      }
    }
    break;
  default:
    break;
  }
}