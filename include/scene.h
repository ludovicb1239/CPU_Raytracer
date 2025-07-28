#ifndef SCENE_H
#define SCENE_H

#include "vector.h"
#include "structs.h"
#include "image.h"

typedef struct {
    VECTOR camera;
    OBJECT* objects;
    int objectCount;
    Image* render;
    float* pixelArray;
} SCENE;

#endif // SCENE_H