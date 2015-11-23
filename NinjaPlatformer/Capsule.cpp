#include "Capsule.h"

void Capsule::init(b2World* world,
                   const glm::vec2& position,
                   const glm::vec2& dimensions,
                   float density,
                   float friction,
                   bool fixedRotation) {
    m_dimensions = dimensions;

    // Make the body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(position.x, position.y);
    bodyDef.fixedRotation = fixedRotation;
    m_body = world->CreateBody(&bodyDef);

    // Create the box
    b2PolygonShape boxShape;
    boxShape.SetAsBox(dimensions.x / 2.0f, (dimensions.y - dimensions.x) / 2.0f);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &boxShape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    m_fixtures[0] = m_body->CreateFixture(&fixtureDef);

    // Create the circles
    b2CircleShape circleShape;
    circleShape.m_radius = dimensions.x / 2.0f;
    
    b2FixtureDef circleDef;
    circleDef.shape = &circleShape;
    circleDef.density = density;
    circleDef.friction = friction;

    // Bottom circle
    circleShape.m_p.Set(0.0f, (-m_dimensions.y + dimensions.x) / 2.0f);
    m_fixtures[1] = m_body->CreateFixture(&circleDef);

    // Top Circle
    circleShape.m_p.Set(0.0f, (m_dimensions.y - dimensions.x) / 2.0f);
    m_fixtures[1] = m_body->CreateFixture(&circleDef);
}

void Capsule::destroy(b2World* world) {
    if (m_body) {
        world->DestroyBody(m_body);
        m_body = nullptr;
    }
}

void Capsule::drawDebug(Bengine::DebugRenderer& debugRenderer) {
    Bengine::ColorRGBA8 color(255, 255, 255, 255);
    // Draw box
    glm::vec4 destRect;
    destRect.x = m_body->GetPosition().x - m_dimensions.x / 2.0f;
    destRect.y = m_body->GetPosition().y - (m_dimensions.y - m_dimensions.x) / 2.0f;
    destRect.z = m_dimensions.x;
    destRect.w = m_dimensions.y - m_dimensions.x;
    debugRenderer.drawBox(destRect, color, m_body->GetAngle());

    // Draw Circle
    debugRenderer.drawCircle(glm::vec2(destRect.x + m_dimensions.x / 2.0f, destRect.y), color, m_dimensions.x / 2.0f);
    debugRenderer.drawCircle(glm::vec2(destRect.x + m_dimensions.x / 2.0f, destRect.y + destRect.w), color, m_dimensions.x / 2.0f);
}