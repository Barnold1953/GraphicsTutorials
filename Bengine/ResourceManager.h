#pragma once
#include "TextureCache.h"
#include <string>

namespace Bengine {

    //This is a way for us to access all our resources, such as
    //Models or textures.
    class ResourceManager
    {
    public:
        static GLTexture getTexture(std::string texturePath);

    private:
        static TextureCache _textureCache;
    };

}