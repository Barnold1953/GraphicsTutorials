#pragma once
#include <map>
#include "GLTexture.h"

namespace Bengine {

    //This caches the textures so that multiple sprites can use the same textures
    class TextureCache
    {
    public:
        TextureCache();
        ~TextureCache();

        GLTexture getTexture(std::string texturePath);

    private:
        std::map<std::string, GLTexture> _textureMap;
    };

}