#ifndef RAW_RENDER_H
#define RAW_RENDER_H

#include "vector.h"
#include "image.h"
#include "rmath.h"

typedef struct{
    int w;
    int h;
    VECTOR* p;
} RAW_RENDER;

RAW_RENDER raw_new(int _w, int _h);

inline void raw_write_pixel(RAW_RENDER r, int j, int i, VECTOR c){ // j along width, i along height
    r.p[j + i * r.w] = c;
}

void raw_delete(RAW_RENDER r);

inline VECTOR raw_read_vector(RAW_RENDER r, int j, int i){
    return ( r.p[j + i * r.w] );
}

inline Pixel raw_read_pixel(RAW_RENDER r, int j, int i){
    VECTOR v = raw_read_vector(r, j, i);
    Pixel p = {
        (unsigned char)clampColor(v.x), 
        (unsigned char)clampColor(v.y), 
        (unsigned char)clampColor(v.z)
    };
    return p;
}

Image* image_from_raw(RAW_RENDER r);
void raw_save_bmp(RAW_RENDER r, const char* file);

#endif // RAW_RENDER_H