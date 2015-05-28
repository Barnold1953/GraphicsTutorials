#pragma once
#include "GLTexture.h"

#include <string>

namespace Bengine {

    //Loads images into GLTextures
    class ImageLoader
    {
    public:
        static GLTexture loadPNG(std::string filePath);
    };

}