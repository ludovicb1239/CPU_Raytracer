#ifndef RAW_RENDER_H
#define RAW_RENDER_H

#include "image.h"

typedef struct
{
    float r, g, b;
} RAW_COLOR;

typedef struct{
    int w;
    int h;
    RAW_COLOR* p;
} RAW_RENDER;

RAW_RENDER raw_new(int _w, int _h);

inline void raw_write_pixel(RAW_RENDER r, int j, int i, RAW_COLOR c){ // j along width, i along height
    r.p[j + i * r.w] = c;
}

void raw_delete(RAW_RENDER r);

inline RAW_COLOR raw_read_vector(RAW_RENDER r, int j, int i){
    return ( r.p[j + i * r.w] );
}

inline Pixel raw_read_pixel(RAW_RENDER r, int j, int i){
    RAW_COLOR v = raw_read_vector(r, j, i);
    Pixel p = {
        (unsigned char)((v.r > 1.0) ? 255 : (v.r * 255)), 
        (unsigned char)((v.g > 1.0) ? 255 : (v.g * 255)), 
        (unsigned char)((v.b > 1.0) ? 255 : (v.b * 255))
    };
    return p;
}

Image* image_from_raw(RAW_RENDER r);
void raw_save_bmp(RAW_RENDER r, const char* file);

#endif // RAW_RENDER_H