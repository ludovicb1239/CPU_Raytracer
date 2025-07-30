#ifndef STRUCTS_H
#define STRUCTS_H

#include "vector.h"

#define PI 3.14159265359

typedef struct{
    VECTOR center;
    float radius;
    VECTOR colorAbsorbtion;
    float metallic;
    float roughness;
    float IOR;
}OBJECT;

#endif // STRUCTS_H