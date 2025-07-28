#ifndef RMATH_H
#define RMATH_H

#include <stdlib.h>

inline float RV(float min, float max){
    float result = (float)rand() / RAND_MAX;
    return ( min + result * (max - min) );
}
inline float clampColor(float d) {
  return ((d > 1.0) ? 255 : (d * 255));
}

#endif // RMATH_H