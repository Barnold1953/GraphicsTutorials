#pragma once
#include <GL/glew.h>
#include "GLTexture.h"

#include <string>

namespace Bengine {

    //A 2D quad that can be rendered to the screen
    class Sprite
    {
    public:
        Sprite();
        ~Sprite();

        void init(float x, float y, float width, float height, std::string texturePath);

        void draw();

        //private:
        float _x;
        float _y;
        float _width;
        float _height;
        GLuint _vboID;
        GLTexture _texture;

    };

}