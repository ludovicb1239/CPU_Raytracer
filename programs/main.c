#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "01_09_02_bmp.h"
#include <time.h>
#include <pthread.h>
#include "OpenImageDenoise/oidn.h"

typedef struct
{
    double x, y, z;
} VECTOR;
typedef struct{
    VECTOR center;
    float radius;
    VECTOR colorAbsorbtion;
    float metallic;
    float roughness;
    float IOR;
}OBJECT;
typedef struct{
    int threadWitdh;
    int threadHeight;
    const VECTOR *camera;
    const float *ratio;
    const float *screen;
    OBJECT *objects;
    const float *stepY;
    const float *stepX;
    Image* render;
    float* pixelArray;
}ThreadInfo;

#define width 2944          //Screen width in pixels
#define height 1664         //Screen height in pixels
#define threadPixelSize 128  //Numbers of pixels in a single thread
#define max_depth 8         //Bounces
#define maxSamples 10      //Number of samples per 1/4 pixel

#define PI 3.14159265359


#define OBJECT_COUNT 9

VECTOR normalize(VECTOR p);
VECTOR reflected(VECTOR vector, VECTOR axis);
float RV(float min, float max);
double dot(VECTOR v, VECTOR u);
VECTOR substractVectors(VECTOR v, VECTOR u);
VECTOR addVectors(VECTOR v, VECTOR u);
VECTOR multiplyVectors(VECTOR v, double u);
VECTOR multiplyTwoVectors(VECTOR v, VECTOR u);
int sphere_intersect(VECTOR center, float radius, VECTOR ray_origin, VECTOR ray_direction, double *dist);
int nearest_intersected_object(OBJECT objects[], VECTOR ray_origin, VECTOR ray_direction, OBJECT *nearest_object, double *min_distance, int objectsCount);
float clampColor(float d);
VECTOR randomVECTOR();
void drawImage(Image *image){};

void denoise(float* image){
    // Allocate memory for float
    float *outData = (float *)malloc(width * height * 3 * sizeof(float));
    // Create an Intel Open Image Denoise device
    OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
    oidnCommitDevice(device);

    OIDNFilter filter = oidnNewFilter(device, "RT"); // generic ray tracing filte
    oidnSetSharedFilterImage(filter, "color", image, OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0);
    oidnSetSharedFilterImage(filter, "output", outData, OIDN_FORMAT_FLOAT3, width, height, 0, 0, 0); // denoised beauty
    //oidnSetFilter1b(filter, "hdr", true); // beauty image is HDR
    oidnCommitFilter(filter);
    // Filter the image
    oidnExecuteFilter(filter);

    // Check for errors
    const char* errorMessage;
    if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE)
      printf("\nError: %s", errorMessage);

    // Cleanup
    oidnReleaseFilter(filter);
    oidnReleaseDevice(device);

    printf("\nSuccessfully denoised the image\n");

    Image* denoisedI = NouvelleImage(width,height);
    for (int i = 0; i < width * height; i++)
    {
        denoisedI->dat[i] = (Pixel){outData[i *3] * 255, outData[i *3 + 1] * 255, outData[i *3 + 2] * 255};
    }
    Sauver(denoisedI,"denoised.bmp");
    DelImage(denoisedI);
}

void *myThreadFun(ThreadInfo* info)
{
    const VECTOR backgroundColor = {0,0,0};
    float y;
    float x;
    VECTOR color;
    VECTOR pixel;
    VECTOR directions[4];
    VECTOR direction;
    VECTOR reflection;

    VECTOR intersections[4];
    VECTOR intersection;
    int typeOfIntersection[4]; //0 normal, 2 sky, 1 bright
    VECTOR normals_to_surface[4];
    VECTOR normal_to_surface;
    OBJECT nearest_objects[4];
    OBJECT nearest_object;
    int insideOfSphere = 0;
    int insidesOfSphere[4];

    int i, j;
    int dx, dy;
    int sampling;
    int ind;
    double min_distance;

    const int startPixelHeight = (info->threadHeight) * threadPixelSize, stopPixelHeight = threadPixelSize * (info->threadHeight + 1);
    const int startPixelWitdh = (info->threadWitdh) * threadPixelSize, stopPixelWitdh = threadPixelSize * (info->threadWitdh + 1);

    for (i = startPixelHeight; i < stopPixelHeight; i++){
        y = (info->screen)[1] + (*(info->stepY)) * i;
        for (j = startPixelWitdh; j < stopPixelWitdh; j++){
            x = (info->screen)[0] + (*(info->stepX)) * j;

            //Emits 4 rays then calculates the intersections
            for (dx = 0; dx < 2; dx++){
                for (dy = 0; dy < 2; dy++){
                    ind = dx*2 + dy;
                    // screen is on origin
                    pixel = (VECTOR){x + dx * (*(info->stepX))/2.0, y + dy * (*(info->stepY))/2.0, 0.0};
                    directions[ind] = normalize(substractVectors(pixel, *info->camera));

                    if (nearest_intersected_object(info->objects, *info->camera, directions[ind], &nearest_object, &min_distance, OBJECT_COUNT) == 0){ //Ray intercepts sphere ?
                        //No sphere is intercepted -> sky
                        typeOfIntersection[ind] = 2;
                    }else{
                        nearest_objects[ind] = nearest_object;
                        if (nearest_objects[ind].colorAbsorbtion.x > 1){
                            //bright color
                            typeOfIntersection[ind] = 1;
                        }else{
                            typeOfIntersection[ind] = 0;
                            intersections[ind] = addVectors(*info->camera, multiplyVectors(directions[ind], min_distance -  ((nearest_object.IOR <= 1.0) ? 0.005 : -.005)));
                            insidesOfSphere[ind] = (nearest_object.IOR <= 1.0) ? 0 : 1;
                            normals_to_surface[ind] = normalize(substractVectors(intersections[ind], nearest_objects[ind].center));
                        }
                    }

                }
            }

            color.x = 0; //r
            color.y = 0; //g
            color.z = 0; //b

            for (dx = 0; dx < 2; dx++){
                for (dy = 0; dy < 2; dy++){
                    ind = dx*2 + dy;

                    if (typeOfIntersection[ind] == 1){
                        //bright color, clamp it
                        color.x += nearest_objects[ind].colorAbsorbtion.x > 1 ? 1 : nearest_objects[ind].colorAbsorbtion.x;
                        color.y += nearest_objects[ind].colorAbsorbtion.y > 1 ? 1 : nearest_objects[ind].colorAbsorbtion.y;
                        color.z += nearest_objects[ind].colorAbsorbtion.z > 1 ? 1 : nearest_objects[ind].colorAbsorbtion.z;

                    }else if (typeOfIntersection[ind] == 2){
                        //sky, return sky color
                        color.x += backgroundColor.x;
                        color.y += backgroundColor.y;
                        color.z += backgroundColor.z;

                    }else{
                        //Set numbers of samples by roughness
                        int samples = nearest_objects[ind].roughness < 0.5 ? (maxSamples/2) : maxSamples;
                        for (sampling = 0; sampling < samples; sampling++) {
                            direction = directions[ind];
                            normal_to_surface = normals_to_surface[ind];
                            intersection = intersections[ind];
                            nearest_object = nearest_objects[ind];
                            insideOfSphere = insidesOfSphere[ind];

                            reflection.x = 1;
                            reflection.y = 1;
                            reflection.z = 1;

                            VECTOR accumColor = {0, 0, 0};
                            for (int k = 0; k < max_depth; k++) {
                                reflection.x *= nearest_object.colorAbsorbtion.x;
                                reflection.y *= nearest_object.colorAbsorbtion.y;
                                reflection.z *= nearest_object.colorAbsorbtion.z;

                                if (reflection.x <= 0.001 && reflection.y <= 0.001 && reflection.z <= 0.001) {
                                    break; // early termination if negligible contribution
                                } else if (reflection.x > 1.0 || reflection.y > 1.0 || reflection.z > 1.0) {
                                    accumColor.x += reflection.x;
                                    accumColor.y += reflection.y;
                                    accumColor.z += reflection.z;
                                    break;
                                }

                                if (nearest_object.IOR > 1.0) {
                                    if (insideOfSphere) {
                                        direction = normalize(substractVectors(direction, multiplyVectors(normal_to_surface, 2)));
                                    } else {
                                        direction = normalize(substractVectors(direction, multiplyVectors(normal_to_surface, 0.5)));
                                    }
                                } else {
                                    if (nearest_object.metallic > 0.5) {
                                        VECTOR randVEC = multiplyVectors(randomVECTOR(), nearest_object.roughness);
                                        direction = normalize(addVectors(reflected(direction, normal_to_surface), randVEC));
                                    } else {
                                        VECTOR randVEC = multiplyVectors(randomVECTOR(), (nearest_object.roughness / 4.0) + 0.75);
                                        direction = normalize(addVectors(normal_to_surface, randVEC));
                                    }
                                }

                                if (!nearest_intersected_object(info->objects, intersection, direction, &nearest_object, &min_distance, OBJECT_COUNT)) {
                                    accumColor.x += reflection.x * backgroundColor.x;
                                    accumColor.y += reflection.y * backgroundColor.y;
                                    accumColor.z += reflection.z * backgroundColor.z;
                                    break;
                                }

                                intersection = addVectors(intersection, multiplyVectors(direction, min_distance - ((nearest_object.IOR <= 1.0) ? 0.005 : -0.005)));
                                insideOfSphere = (nearest_object.IOR <= 1.0) ? 0 : 1;
                                normal_to_surface = normalize(substractVectors(intersection, nearest_object.center));
                            }

                            color.x += accumColor.x / samples;
                            color.y += accumColor.y / samples;
                            color.z += accumColor.z / samples;

                            // Optional: If color converges early, break out of the sampling loop
                            if (sampling >= 4) {
                                float avg = (color.x + color.y + color.z) / 3.0f;
                                if (avg > 0.95) break;
                            }
                        }

                    }
                }
            }
            color = multiplyVectors(color, 0.25);
            color.x = sqrt(color.x);
            color.y = sqrt(color.y);
            color.z = sqrt(color.z);
            info->pixelArray[3 * (j + i * width)] = color.x;
            info->pixelArray[3 * (j + i * width) + 1] = color.y;
            info->pixelArray[3 * (j + i * width) + 2] = color.z;
            SetPixelBMP(info->render,j,i,(Pixel) {(unsigned char)clampColor(color.x), (unsigned char)clampColor(color.y), (unsigned char)clampColor(color.z)});
        }
    }
    fprintf(stderr, "Made\t%d\t%d \r", info->threadWitdh, info->threadHeight);
    pthread_exit(NULL);
    return NULL;
}

int main(int argc, char *argv[])
{
    const int widthThreads = width/threadPixelSize;
    const int heightThreads = height/threadPixelSize;

    const VECTOR camera = {0, 0, 1};  //Camera pos ?

    const float ratio = (float)width / (float)height;
    float screen[4] = {-1, 1.0 / ratio, 1, -1.0 / ratio}; // left, top, right, bottom

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

    //Create a new image
    Image* render = NouvelleImage(width,height);
    float *pixelArray = (float *)malloc(width * height * 3 * sizeof(float));

    const float stepY = (screen[3] - screen[1]) / (float)(height - 1);
    const float stepX = (screen[2] - screen[0]) / (float)(width - 1);
    int i,j = 0;

    srand(2465); //random seed

    /*
    for (i = 2; i < 40; i++){
        float rad = RV(0.4, 1.3);
        VECTOR baseColor = newVector(RV(.3, 1), RV(.3, 1), RV(.3, 1));
        objects[i] = newObject(newVector(RV(-8, 8), rad - 1, RV(-20, -2)), rad, baseColor, RV(0, 1), RV(0, 1));
    }*/


    printf("Starting render ...\n");
    clock_t begin = clock();

    pthread_t threads[widthThreads * heightThreads];
    ThreadInfo info[widthThreads * heightThreads];
    int n = 0;
    for (i = 0; i < widthThreads; i++){
        for (j = 0; j < heightThreads; j++){
            info[n] = (ThreadInfo){i, j, &camera, &ratio, screen, objects, &stepY, &stepX, render, pixelArray};
            pthread_create(&threads[n], NULL, &myThreadFun, &info[n]);
            n ++;
        }
    }
    for (n = 0; n < widthThreads * heightThreads; n++) {
        pthread_join(threads[n], NULL); //Waiting for every thread to be done
    }

    clock_t end = clock();
    unsigned long millis = (end -  begin) * 1000 / CLOCKS_PER_SEC;
    printf("\nFinished in %ld ms", millis );

    Sauver(render,"render.bmp");

    printf("\nSaved Raw Render");

    denoise(pixelArray);

    drawImage(render);

    DelImage(render);

    printf("\nSaved Denoised Render");

    getchar();

    return 0;
}
inline VECTOR normalize(VECTOR p){
    const float squared = (p.x*p.x + p.y*p.y + p.z*p.z);

	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = squared * 0.5F;
	y = squared;
	i = * ( long * ) &y; // evil floating point bit level hacking
	i = 0x5f3759df - ( i >> 1 ); // what the fuck?
	y = * ( float * ) &i;
	y = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration

    VECTOR vectorReturn;
    vectorReturn.x = p.x * y;
    vectorReturn.y = p.y * y;
    vectorReturn.z = p.z * y;

    return vectorReturn;
}
inline VECTOR randomVECTOR(){
    VECTOR newVec;
    newVec.x = RV(-1.0,1.0);
    newVec.y = RV(-1.0,1.0);
    newVec.z = RV(-1.0,1.0);
    return (normalize(newVec));
}
inline float RV(float min, float max){
    float result = (float)rand() / RAND_MAX;
    return ( min + result * (max - min) );
}
inline VECTOR reflected(VECTOR vector, VECTOR axis){
    return substractVectors(vector, multiplyVectors(axis, 2.0 * dot(vector, axis)));
}
inline VECTOR substractVectors(VECTOR v, VECTOR u){
    VECTOR vectorReturn;
    vectorReturn.x = v.x - u.x;
    vectorReturn.y = v.y - u.y;
    vectorReturn.z = v.z - u.z;
    return vectorReturn;
}
inline VECTOR addVectors(VECTOR v, VECTOR u){
    VECTOR vectorReturn;
    vectorReturn.x = v.x + u.x;
    vectorReturn.y = v.y + u.y;
    vectorReturn.z = v.z + u.z;
    return vectorReturn;
}
inline VECTOR multiplyVectors(VECTOR v, double u){
    VECTOR vectorReturn;
    vectorReturn.x = v.x * u;
    vectorReturn.y = v.y * u;
    vectorReturn.z = v.z * u;
    return vectorReturn;
}
inline VECTOR multiplyTwoVectors(VECTOR v, VECTOR u){
    VECTOR vectorReturn;
    vectorReturn.x = v.x * u.x;
    vectorReturn.y = v.y * u.y;
    vectorReturn.z = v.z * u.z;
    return vectorReturn;
}
inline double dot(VECTOR v, VECTOR u){
    return v.x*u.x + v.y*u.y + v.z*u.z;
}
inline float clampColor(float d) {
  return ((d > 1.0) ? 255 : (d * 255));
}
inline int sphere_intersect(VECTOR center, float radius, VECTOR ray_origin, VECTOR ray_direction, double *dist){
    VECTOR m = substractVectors(ray_origin, center);
    double b = dot(ray_direction, m);
    double c = dot(m, m) - (double)radius*radius;

    if (c > 0.0f && b > 0.0f) return 0;

    double delta = b*b - c;

    if (delta >= 0.0){
        double mSqrt = sqrt(delta);
        double t1 = -b - mSqrt;
        if (t1 > 0.0){
            *dist = t1;
            return 1;
        }
        /*
        float t2 = -b + mSqrt;
        if (t1 > 0.0f || t2 > 0.0f){
            if (t1 < t2 && t1 > 0.0f){
                *dist = t1;
                return 1;
            }/*
            else if (t2 > 0.0f){
                *dist = t2;
                return 1;
            }
        }*/
    }
    return 0;
}
inline int nearest_intersected_object (OBJECT objects[], VECTOR ray_origin, VECTOR ray_direction, OBJECT *nearest_object, double *min_distance, int objectsCount){
    *min_distance = 1e9;
    double distance;
    int found = 0;
    for (int i = 0; i < objectsCount; i++){
        if (sphere_intersect(objects[i].center, objects[i].radius, ray_origin, ray_direction, &distance) == 1){
            if (distance < *min_distance){
                *min_distance = distance;
                *nearest_object = objects[i];
                found = 1;
            }
        }
    }
    return found;
}
