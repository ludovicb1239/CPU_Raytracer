#ifndef STRUCTS_H
#define STRUCTS_H

#include "vector.h"
#include "raw_render.h"

#define PI 3.14159265359

typedef struct{
    VECTOR center;
    float radius;
    RAW_COLOR colorAbsorbtion;
    float metallic;
    float roughness;
    float IOR;
}OBJECT;

#endif // STRUCTS_H