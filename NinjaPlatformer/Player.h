#pragma once

#include "Capsule.h"
#include <Bengine/InputManager.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLTexture.h>

class Player {
public:
    void init(b2World* world,
              const glm::vec2& position,
              const glm::vec2& drawDims,
              glm::vec2& collisionDims,
              Bengine::ColorRGBA8 color);

    void draw(Bengine::SpriteBatch& spriteBatch);
    void drawDebug(Bengine::DebugRenderer& debugRenderer);

    void update(Bengine::InputManager& inputManager);

    const Capsule& getCapsule() const { return m_capsule; }

private:
    glm::vec2 m_drawDims;
    Bengine::ColorRGBA8 m_color;
    Bengine::GLTexture m_texture;
    Capsule m_capsule;
};

