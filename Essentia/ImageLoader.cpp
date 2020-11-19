#include "pch.h"
#include "ImageLoader.h"
#include "lodepng.h"

// Loads PNG image 
// Reference: https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
std::vector<unsigned char> es::image::LoadPngImage(const char* imageFile, uint32& outWidth, uint32& outHeight)
{
    std::vector<unsigned char> image; //the raw pixels
    //decode
    unsigned error = lodepng::decode(image, outWidth, outHeight, imageFile);

    //if there's an error, display it
    if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return image;
}
