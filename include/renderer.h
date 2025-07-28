#ifndef RENDERER_H
#define RENDERER_H

#include "scene.h"

typedef struct {
    int max_depth; // Maximum depth of ray tracing
    int max_samples; // Maximum number of samples per pixel
    int widthThreads; // Number of threads in width
    int heightThreads; // Number of threads in height
    int threadPixelSize; // Number of pixels wide in a single thread
} RENDER_PARAMS;

void render_scene(SCENE *scene, RENDER_PARAMS params);

#endif