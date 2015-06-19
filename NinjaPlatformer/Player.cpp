#include "Player.h"
#include <Bengine/ResourceManager.h>
#include <SDL/SDL.h>
#include <iostream>

void Player::init(b2World* world,
                  const glm::vec2& position,
                  const glm::vec2& drawDims,
                  glm::vec2& collisionDims,
                  Bengine::ColorRGBA8 color) {
    Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Assets/blue_ninja.png");
    m_color = color;
    m_drawDims = drawDims;
    m_capsule.init(world, position, collisionDims, 1.0f, 0.1f, true);
    m_texture.init(texture, glm::ivec2(10, 2));
}

void Player::draw(Bengine::SpriteBatch& spriteBatch) {
    glm::vec4 destRect;
    b2Body* body = m_capsule.getBody();
    destRect.x = body->GetPosition().x - m_drawDims.x / 2.0f;
    destRect.y = body->GetPosition().y - m_capsule.getDimensions().y / 2.0f;
    destRect.z = m_drawDims.x;
    destRect.w = m_drawDims.y;

    int tileIndex;
    int numTiles;

    float animSpeed = 0.2f;
    glm::vec2 velocity;
    velocity.x = body->GetLinearVelocity().x;
    velocity.y = body->GetLinearVelocity().y;

    // Calculate animation
    if (m_onGround) {
       
        if (m_isPunching) {
            numTiles = 4;
            tileIndex = 1;
            if (m_moveState != PlayerMoveState::PUNCHING) {
                m_moveState = PlayerMoveState::PUNCHING;
                m_animTime = 0.0f;
            }
        } else if (abs(velocity.x) > 1.0f && ((velocity.x > 0 && m_direction > 0) || (velocity.x < 0 && m_direction < 0))) {
            // Running
            numTiles = 6;
            tileIndex = 10;
            animSpeed = abs(velocity.x) * 0.025f;
            if (m_moveState != PlayerMoveState::RUNNING) {
                m_moveState = PlayerMoveState::RUNNING;
                m_animTime = 0.0f;
            }
        } else {
            // Standing still
            numTiles = 1;
            tileIndex = 0;
            m_moveState = PlayerMoveState::STANDING;
        }

    } else {
        // In the air
        if (m_isPunching) {
            numTiles = 1;
            tileIndex = 18;
            animSpeed *= 0.25f;
            if (m_moveState != PlayerMoveState::PUNCHING) {
                m_moveState = PlayerMoveState::PUNCHING;
                m_animTime = 0.0f;
            }
        } else if (abs(velocity.x) > 10.0f) {
            numTiles = 1;
            tileIndex = 10;
            m_moveState = PlayerMoveState::IN_AIR;
        } else if (velocity.y <= 0.0f) {
            // Falling
            numTiles = 1;
            tileIndex = 17;
            m_moveState = PlayerMoveState::IN_AIR;
        } else {
            // Rising
            numTiles = 1;
            tileIndex = 16;
            m_moveState = PlayerMoveState::IN_AIR;
        }
    }

    // Increment animation time
    m_animTime += animSpeed;

    // Check for punch end
    if (m_animTime > numTiles) {
        m_isPunching = false;
    }

    // Apply animation
    tileIndex = tileIndex + (int)m_animTime % numTiles;

    // Get the uv coordinates from the tile index
    glm::vec4 uvRect = m_texture.getUVs(tileIndex);

    // Check direction
    if (m_direction == -1) {
        uvRect.x += 1.0f / m_texture.dims.x;
        uvRect.z *= -1;
    }

    // Draw the sprite
    spriteBatch.draw(destRect, uvRect, m_texture.texture.id, 0.0f, m_color, body->GetAngle());
}

void Player::drawDebug(Bengine::DebugRenderer& debugRenderer) {
    m_capsule.drawDebug(debugRenderer);
}

void Player::update(Bengine::InputManager& inputManager) {
    b2Body* body = m_capsule.getBody();
    if (inputManager.isKeyDown(SDLK_a)) {
        body->ApplyForceToCenter(b2Vec2(-100.0, 0.0), true);
        m_direction = -1;
    } else if (inputManager.isKeyDown(SDLK_d)) {
        body->ApplyForceToCenter(b2Vec2(100.0, 0.0), true);
        m_direction = 1;
    } else {
        body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x * 0.95, body->GetLinearVelocity().y));
    }

    // Check for punch
    if (inputManager.isKeyPressed(SDLK_SPACE)) {
        m_isPunching = true;
    }

    float MAX_SPEED = 10.0f;
    if (body->GetLinearVelocity().x < -MAX_SPEED) {
        body->SetLinearVelocity(b2Vec2(-MAX_SPEED, body->GetLinearVelocity().y));
    } else if (body->GetLinearVelocity().x > MAX_SPEED) {
        body->SetLinearVelocity(b2Vec2(MAX_SPEED, body->GetLinearVelocity().y));
    }

    // Loop through all the contact points
    m_onGround = false;
    for (b2ContactEdge* ce = body->GetContactList(); ce != nullptr; ce = ce->next) {
        b2Contact* c = ce->contact;
        if (c->IsTouching()) {
            b2WorldManifold manifold;
            c->GetWorldManifold(&manifold);
            // Check if the points are below
            bool below = false;
            for (int i = 0; i < b2_maxManifoldPoints; i++) {
                if (manifold.points[i].y < body->GetPosition().y - m_capsule.getDimensions().y / 2.0f + m_capsule.getDimensions().x / 2.0f + 0.01f) {
                    below = true;
                    break;
                }
            }
            if (below) {
                m_onGround = true;
                // We can jump
                if (inputManager.isKeyPressed(SDLK_w)) {
                    body->ApplyLinearImpulse(b2Vec2(0.0f, 30.0), b2Vec2(0.0f, 0.0f), true);
                    break;
                }
            }
        }
    }
}
