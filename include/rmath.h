#ifndef RMATH_H
#define RMATH_H

#include <stdlib.h>

inline float RV(float min, float max){
    float result = (float)rand() / RAND_MAX;
    return ( min + result * (max - min) );
}

#endif // RMATH_H