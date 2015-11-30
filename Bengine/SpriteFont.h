/* 
    This is a modified version of the SpriteFont class from the
    Seed Of Andromeda source code.
    Use it for any of your projects, commercial or otherwise,
    free of charge, but do not remove this disclaimer.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS
    ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
    EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
    INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
    RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
    ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
    OF THIS SOFTWARE.

    November 28 2014
    Original Author: Cristian Zaloj
    Modified By: Benjamin Arnold
*/

#pragma once

#ifndef SpriteFont_h__
#define SpriteFont_h__

#include <TTF/SDL_ttf.h>
#include <glm/glm.hpp>
#include <map>
#include <vector>

#include "Vertex.h"

namespace Bengine {

    struct GLTexture;
    class SpriteBatch;

    struct CharGlyph {
    public:
        char character;
        glm::vec4 uvRect;
        glm::vec2 size;
    };

#define FIRST_PRINTABLE_CHAR ((char)32)
#define LAST_PRINTABLE_CHAR ((char)126)

    /// For text justification
    enum class Justification {
        LEFT, MIDDLE, RIGHT
    };

    class SpriteFont {
    public:
        SpriteFont() {};
        SpriteFont(const char* font, int size, char cs, char ce);
        SpriteFont(const char* font, int size) :
            SpriteFont(font, size, FIRST_PRINTABLE_CHAR, LAST_PRINTABLE_CHAR) {
        }

        void init(const char* font, int size);
        void init(const char* font, int size, char cs, char ce);

        /// Destroys the font resources
        void dispose();

        int getFontHeight() const {
            return _fontHeight;
        }

        /// Measures the dimensions of the text
        glm::vec2 measure(const char* s);

        /// Draws using a spritebatch
        void draw(SpriteBatch& batch, const char* s, glm::vec2 position, glm::vec2 scaling, 
                  float depth, ColorRGBA8 tint, Justification just = Justification::LEFT);
    private:
        static std::vector<int>* createRows(glm::ivec4* rects, int rectsLength, int r, int padding, int& w);

        int _regStart, _regLength;
        CharGlyph* _glyphs;
        int _fontHeight;

        unsigned int _texID;
    };

}

#endif // SpriteFont_h__