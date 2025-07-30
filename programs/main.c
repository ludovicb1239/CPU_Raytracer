#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "image.h"
#include "structs.h"
#include "renderer.h"
#include "denoiser.h"


#define THREAD_PIXEL_SIZE 256  //Numbers of pixels in a single thread
#define WIDTH_THREADS 16          // Number of threads in width
#define HEIGHT_THREADS 9         // Number of threads in height
#define MAX_DEPTH 10         //Bounces
#define MAX_SAMPLES 80      //Number of samples per 1/4 pixel

#define OBJECT_COUNT 9

int main(int argc, char *argv[])
{
    int width = WIDTH_THREADS * THREAD_PIXEL_SIZE;          //Screen width in pixels
    int height = HEIGHT_THREADS * THREAD_PIXEL_SIZE;         //Screen height in pixels


    //OBJECTS
    OBJECT objects[OBJECT_COUNT];
    objects[0] = (OBJECT){{0,-90000, 0}, 90000 - 2, {.8, .8, .8},    1, .05, 0};
    objects[1] = (OBJECT){{0, 90000, 0}, 90000 - 2, {.95, .95, .95}, 0, 1, 0};
    objects[2] = (OBJECT){{-90000,0, 0}, 90000 - 2, {1, .1, .1},     0, 1, 0};
    objects[3] = (OBJECT){{90000, 0, 0}, 90000 - 2, {.1, .1, 1},     0, 1, 0};
    objects[4] = (OBJECT){{0, 0,-90000}, 90000 - 6, {.95, .95, .95}, 0, 1, 0};
    objects[5] = (OBJECT){{0, 0, 90000}, 90000 - 1.1, {.1, .1, .1},  0, 1, 0};
    objects[6] = (OBJECT){{0, 20, -4}  , 18.05,     {5, 5, 5},       0, 1, 0};
    objects[7] = (OBJECT){{1, -1.2, -4}, .8,        {.95, .95, .95}, 0, .9, 0};
    objects[8] = (OBJECT){{-1, -1, -5} , 1.0,       {.85, .85, .85}, 1, .01, 0};

    //  Y
    //  |
    //  |
    //  Z---X


    /*
    for (i = 2; i < 40; i++){
        float rad = RV(0.4, 1.3);
        VECTOR baseColor = newVector(RV(.3, 1), RV(.3, 1), RV(.3, 1));
        objects[i] = newObject(newVector(RV(-8, 8), rad - 1, RV(-20, -2)), rad, baseColor, RV(0, 1), RV(0, 1));
    }*/


    SCENE scene = {
        .camera = {0, 0, 1},  //Camera pos
        .objects = objects,
        .objectCount = OBJECT_COUNT
    };
    RENDER_PARAMS params = {
        .max_depth = MAX_DEPTH,
        .max_samples = MAX_SAMPLES,
        .widthThreads = WIDTH_THREADS,
        .heightThreads = HEIGHT_THREADS,
        .threadPixelSize = THREAD_PIXEL_SIZE
    };

    printf("Starting render ...\n");
    clock_t begin = clock();
    
    RAW_RENDER render = render_scene(&scene, params);

    clock_t end = clock();
    unsigned long millis = (end -  begin) * 1000 / CLOCKS_PER_SEC;
    printf("\nFinished in %ld ms", millis );

    raw_save_bmp(render, "render.bmp");

    denoise(render);
    raw_save_bmp(render, "denoised.bmp");

    raw_delete(render);

    return 0;
}