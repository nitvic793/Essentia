#pragma once
#include <vector>
#include "Declarations.h"

namespace es
{
	namespace image
	{
		std::vector<unsigned char> LoadPngImage(const char* imageFile, uint32& outWidth, uint32& outHeight);
	}
}