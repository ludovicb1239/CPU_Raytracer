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
    RAW_RENDER render;
    float* screen;
    float stepY;
    float stepX;
}ThreadInfo;
 

int sphere_intersect(VECTOR center, float radius, VECTOR ray_origin, VECTOR ray_direction, double *dist);
int nearest_intersected_object(OBJECT objects[], VECTOR ray_origin, VECTOR ray_direction, OBJECT *nearest_object, double *min_distance, int objectsCount);

static int threads_done;

void *myThreadFun(ThreadInfo* info)
{
    const RAW_COLOR backgroundColor = {0,0,0};
    float y;
    float x;
    RAW_COLOR color;
    VECTOR pixel;
    VECTOR directions[4];
    VECTOR direction;
    RAW_COLOR reflection;

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

    int width = info->render.w;
    int height = info->render.h;

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
                        if (nearest_objects[ind].colorAbsorbtion.r > 1){
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

            color.r = 0; //r
            color.g = 0; //g
            color.b = 0; //b

            for (dx = 0; dx < 2; dx++){
                for (dy = 0; dy < 2; dy++){
                    ind = dx*2 + dy;

                    if (typeOfIntersection[ind] == 1){
                        //bright color, clamp it
                        color.r += nearest_objects[ind].colorAbsorbtion.r > 1 ? 1 : nearest_objects[ind].colorAbsorbtion.r;
                        color.g += nearest_objects[ind].colorAbsorbtion.g > 1 ? 1 : nearest_objects[ind].colorAbsorbtion.g;
                        color.b += nearest_objects[ind].colorAbsorbtion.b > 1 ? 1 : nearest_objects[ind].colorAbsorbtion.b;

                    }else if (typeOfIntersection[ind] == 2){
                        //sky, return sky color
                        color.r += backgroundColor.r;
                        color.g += backgroundColor.g;
                        color.b += backgroundColor.b;

                    }else{
                        //Set numbers of samples by roughness
                        int samples = nearest_objects[ind].roughness < 0.5 ? (info->params.max_samples/2) : info->params.max_samples;
                        for (sampling = 0; sampling < samples; sampling++) {
                            direction = directions[ind];
                            normal_to_surface = normals_to_surface[ind];
                            intersection = intersections[ind];
                            nearest_object = nearest_objects[ind];
                            insideOfSphere = insidesOfSphere[ind];

                            reflection.r = 1;
                            reflection.g = 1;
                            reflection.b = 1;

                            RAW_COLOR accumColor = {0, 0, 0};
                            for (int k = 0; k < info->params.max_depth; k++) {
                                reflection.r *= nearest_object.colorAbsorbtion.r;
                                reflection.g *= nearest_object.colorAbsorbtion.g;
                                reflection.b *= nearest_object.colorAbsorbtion.b;

                                if (reflection.r <= 0.001 && reflection.g <= 0.001 && reflection.b <= 0.001) {
                                    break; // early termination if negligible contribution
                                } else if (reflection.r > 1.0 || reflection.g > 1.0 || reflection.b > 1.0) {
                                    accumColor.r += reflection.r;
                                    accumColor.g += reflection.g;
                                    accumColor.b += reflection.b;
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
                                    accumColor.r += reflection.r * backgroundColor.r;
                                    accumColor.g += reflection.g * backgroundColor.g;
                                    accumColor.b += reflection.b * backgroundColor.b;
                                    break;
                                }

                                intersection = addVectors(intersection, multiplyVectors(direction, min_distance - ((nearest_object.IOR <= 1.0) ? 0.005 : -0.005)));
                                insideOfSphere = (nearest_object.IOR <= 1.0) ? 0 : 1;
                                normal_to_surface = normalize(substractVectors(intersection, nearest_object.center));
                            }

                            color.r += accumColor.r / samples;
                            color.g += accumColor.g / samples;
                            color.b += accumColor.b / samples;

                            // Optional: If color converges early, break out of the sampling loop
                            if (sampling >= 4) {
                                float avg = (color.r + color.g + color.b) / 3.0f;
                                if (avg > 0.95) break;
                            }
                        }

                    }
                }
            }
            color.r = sqrt(color.r/4.0f);
            color.g = sqrt(color.g/4.0f);
            color.b = sqrt(color.b/4.0f);
            raw_write_pixel(info->render, j, i, color);
        }
    }
    threads_done++;
    fprintf(stderr, "Made\t%d\t/\t%d \r", threads_done, info->params.widthThreads * info->params.heightThreads);
    pthread_exit(NULL);
    return NULL;
}

RAW_RENDER render_scene(SCENE *scene, RENDER_PARAMS params){
    srand(2465); //random seed
    
    int width = params.widthThreads * params.threadPixelSize;
    int height = params.heightThreads * params.threadPixelSize;

    RAW_RENDER m_render = raw_new(width, height);

    float ratio = (float)width / (float)height;
    float screen[4] = {-1, 1.0 / ratio, 1, -1.0 / ratio}; // left, top, right, bottom
    float stepY = (screen[3] - screen[1]) / (float)(height - 1);
    float stepX = (screen[2] - screen[0]) / (float)(width - 1);

    int i,j = 0;
    int totalThreads = params.widthThreads * params.heightThreads;
    pthread_t threads[totalThreads];
    ThreadInfo info[totalThreads];
    int n = 0;
    threads_done = 0;
    for (i = 0; i < params.widthThreads; i++){
        for (j = 0; j < params.heightThreads; j++){
            info[n] = (ThreadInfo){
                .threadWitdh = i,
                .threadHeight = j,
                .scene = scene,
                .params = params,
                .screen = screen,
                .render = m_render,
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
    return m_render;
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