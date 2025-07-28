#include "denoiser.h"
#include "OpenImageDenoise/oidn.h"

void denoise(float* image){
/*

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
*/

}