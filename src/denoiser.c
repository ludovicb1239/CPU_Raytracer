#include "denoiser.h"
#include "OpenImageDenoise/oidn.h"

#include "raw_render.h"
#include <stdio.h>

void denoise(RAW_RENDER r){
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
    const char* errorMessage;
    if (oidnGetDeviceError(device, &errorMessage) != OIDN_ERROR_NONE)
      printf("\nError: %s", errorMessage);

    // Cleanup
    oidnReleaseFilter(filter);
    oidnReleaseDevice(device);

    RAW_RENDER t = r;
    r = out_render;
    raw_delete(t); // Delete the noisy input

    printf("\nSuccessfully denoised the image\n");
}