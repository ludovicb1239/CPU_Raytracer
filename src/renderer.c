#include "renderer.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef struct{
    int threadWitdh;
    int threadHeight;
    SCENE *scene;
    RENDER_PARAMS params;
    float* screen;
    float stepY;
    float stepX;
}ThreadInfo;


int sphere_intersect(VECTOR center, float radius, VECTOR ray_origin, VECTOR ray_direction, double *dist);
int nearest_intersected_object(OBJECT objects[], VECTOR ray_origin, VECTOR ray_direction, OBJECT *nearest_object, double *min_distance, int objectsCount);

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

    int startPixelHeight = info->params.threadPixelSize * (info->threadHeight);
    int stopPixelHeight = info->params.threadPixelSize * (info->threadHeight + 1);
    int startPixelWitdh = info->params.threadPixelSize * (info->threadWitdh);
    int stopPixelWitdh = info->params.threadPixelSize * (info->threadWitdh + 1);

    int width = info->params.widthThreads * info->params.threadPixelSize;
    int height = info->params.heightThreads * info->params.threadPixelSize;

    for (i = startPixelHeight; i < stopPixelHeight; i++){
        y = (info->screen)[1] + info->stepY * i;
        for (j = startPixelWitdh; j < stopPixelWitdh; j++){
            x = (info->screen)[0] + info->stepX * j;

            //Emits 4 rays then calculates the intersections
            for (dx = 0; dx < 2; dx++){
                for (dy = 0; dy < 2; dy++){
                    ind = dx*2 + dy;
                    // screen is on origin
                    pixel = (VECTOR){x + dx * info->stepX/2.0, y + dy * info->stepY/2.0, 0.0};
                    directions[ind] = normalize(substractVectors(pixel, info->scene->camera));

                    if (nearest_intersected_object(info->scene->objects, info->scene->camera, directions[ind], &nearest_object, &min_distance, info->scene->objectCount) == 0){ //Ray intercepts sphere ?
                        //No sphere is intercepted -> sky
                        typeOfIntersection[ind] = 2;
                    }else{
                        nearest_objects[ind] = nearest_object;
                        if (nearest_objects[ind].colorAbsorbtion.x > 1){
                            //bright color
                            typeOfIntersection[ind] = 1;
                        }else{
                            typeOfIntersection[ind] = 0;
                            intersections[ind] = addVectors(info->scene->camera, multiplyVectors(directions[ind], min_distance -  ((nearest_object.IOR <= 1.0) ? 0.005 : -.005)));
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
                        int samples = nearest_objects[ind].roughness < 0.5 ? (info->params.max_samples/2) : info->params.max_samples;
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
                            for (int k = 0; k < info->params.max_depth; k++) {
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

                                if (!nearest_intersected_object(info->scene->objects, intersection, direction, &nearest_object, &min_distance, info->scene->objectCount)) {
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
            info->scene->pixelArray[3 * (j + i * width)] = color.x;
            info->scene->pixelArray[3 * (j + i * width) + 1] = color.y;
            info->scene->pixelArray[3 * (j + i * width) + 2] = color.z;
            SetPixelBMP(info->scene->render,j,i,(Pixel) {(unsigned char)clampColor(color.x), (unsigned char)clampColor(color.y), (unsigned char)clampColor(color.z)});
        }
    }
    fprintf(stderr, "Made\t%d\t%d \r", info->threadWitdh, info->threadHeight);
    pthread_exit(NULL);
    return NULL;
}

void render_scene(SCENE *scene, RENDER_PARAMS params){
    srand(2465); //random seed
    
    int width = params.widthThreads * params.threadPixelSize;
    int height = params.heightThreads * params.threadPixelSize;


    float ratio = (float)width / (float)height;
    float screen[4] = {-1, 1.0 / ratio, 1, -1.0 / ratio}; // left, top, right, bottom
    float stepY = (screen[3] - screen[1]) / (float)(height - 1);
    float stepX = (screen[2] - screen[0]) / (float)(width - 1);

    int i,j = 0;
    int totalThreads = params.widthThreads * params.heightThreads;
    pthread_t threads[totalThreads];
    ThreadInfo info[totalThreads];
    float *pixelArray = (float *)malloc(width * height * 3 * sizeof(float));
    int n = 0;
    for (i = 0; i < params.widthThreads; i++){
        for (j = 0; j < params.heightThreads; j++){
            info[n] = (ThreadInfo){
                .threadWitdh = i,
                .threadHeight = j,
                .scene = scene,
                .params = params,
                .screen = screen,
                .stepY = stepY,
                .stepX = stepX
            };
            pthread_create(&threads[n], NULL, &myThreadFun, &info[n]);
            n ++;
        }
    }
    for (n = 0; n < totalThreads; n++) {
        pthread_join(threads[n], NULL); //Waiting for every thread to be done
    }
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