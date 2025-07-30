#include "raw_render.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

RAW_RENDER raw_new(int _w, int _h){
    assert(_w > 0 && _h > 0);
    VECTOR* _p = (VECTOR *)malloc(_w * _h * sizeof(VECTOR));
    RAW_RENDER r = {
        .w = _w,
        .h = _h,
        .p = _p
    };
    return r;
}

void raw_delete(RAW_RENDER r){
    free(r.p);
}

Image* image_from_raw(RAW_RENDER r){
    Image* h = NouvelleImage(r.w, r.h);
    for (int i = 0; i < r.h; i++){
        for (int j = 0; j < r.w; j++){
            Pixel p = raw_read_pixel(r, j, i);
            SetPixelBMP(h, j, i, p);
        }
    }
    return h;
}

void raw_save_bmp(RAW_RENDER r, const char* file){
    Image* final_bpm = image_from_raw(r);
    Sauver(final_bpm, file);
    printf("\nSaved BMP Render");
    DelImage(final_bpm);
}