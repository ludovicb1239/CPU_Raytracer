#include "denoiser.h"
#include "OpenImageDenoise/oidn.h"

#include "raw_render.h"
#include <stdio.h>
#include <stdlib.h>

RAW_RENDER denoise(RAW_RENDER r){
    // Allocate memory for float
    RAW_RENDER out_render = raw_new(r.w, r.h);
    // Create an Intel Open Image Denoise device
    OIDNDevice device = oidnNewDevice(OIDN_DEVICE_TYPE_DEFAULT);
    oidnCommitDevice(device);

    OIDNFilter filter = oidnNewFilter(device, "RT"); // generic ray tracing filter
    oidnSetSharedFilterImage(filter, "color", r.p, OIDN_FORMAT_FLOAT3, r.w, r.h, 0, 0, 0);
    oidnSetSharedFilterImage(filter, "output", out_render.p, OIDN_FORMAT_FLOAT3, r.w, r.h, 0, 0, 0); // denoised beauty
    //oidnSetFilter1b(filter, "hdr", true); // beauty image is HDR
    oidnCommitFilter(filter);
    // Filter the image
    oidnExecuteFilter(filter);

    // Check for errors
    const char* errorMessage = NULL;
    if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE) {
        printf("Device error: %s\n", errorMessage);
        exit(1);
    }

    // Cleanup
    oidnReleaseFilter(filter);
    oidnReleaseDevice(device);

    printf("Successfully denoised the image\n");

    return out_render;
}