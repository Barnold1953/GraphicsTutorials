#include "Player.h"
#include <Bengine/ResourceManager.h>
#include <SDL/SDL.h>
#include <iostream>

Player::Player() {
}

Player::~Player() {
}

void Player::init(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::ColorRGBA8 color) {
    Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Assets/blue_ninja.png");
    m_collisionBox.init(world, position, dimensions, texture, color, true, glm::vec4(0.0f, 0.0f, 0.1f, 0.5f));
}

void Player::draw(Bengine::SpriteBatch& spriteBatch) {
    m_collisionBox.draw(spriteBatch);
}

void Player::update(Bengine::InputManager& inputManager) {
    b2Body* body = m_collisionBox.getBody();
    if (inputManager.isKeyDown(SDLK_a)) {
        body->ApplyForceToCenter(b2Vec2(-100.0, 0.0), true);
    } else if (inputManager.isKeyDown(SDLK_d)) {
        body->ApplyForceToCenter(b2Vec2(100.0, 0.0), true);
    } else {
        body->SetLinearVelocity(b2Vec2(body->GetLinearVelocity().x * 0.95, body->GetLinearVelocity().y));
    }

    float MAX_SPEED = 10.0f;
    if (body->GetLinearVelocity().x < -MAX_SPEED) {
        body->SetLinearVelocity(b2Vec2(-MAX_SPEED, body->GetLinearVelocity().y));
    } else if (body->GetLinearVelocity().x > MAX_SPEED) {
        body->SetLinearVelocity(b2Vec2(MAX_SPEED, body->GetLinearVelocity().y));
    }

    // Loop through all the contact points
    for (b2ContactEdge* ce = body->GetContactList(); ce != nullptr; ce = ce->next) {
        b2Contact* c = ce->contact;
        if (c->IsTouching()) {
            b2WorldManifold manifold;
            c->GetWorldManifold(&manifold);
            // Check if the points are below
            bool below = false;
            for (int i = 0; i < b2_maxManifoldPoints; i++) {
                if (manifold.points[i].y < body->GetPosition().y - m_collisionBox.getDimensions().y / 2.0f + 0.01f) {
                    below = true;
                    break;
                }
            }
            if (below) {
                // We can jump
                if (inputManager.isKeyPressed(SDLK_w)) {
                    body->ApplyLinearImpulse(b2Vec2(0.0f, 30.0), b2Vec2(0.0f, 0.0f), true);
                    break;
                }
            }
        }
    }
}
