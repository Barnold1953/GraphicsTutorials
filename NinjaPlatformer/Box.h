#pragma once

#include <Box2D/Box2D.h>
#include <glm/glm.hpp>
#include <Bengine/Vertex.h>

class Box {
public:
    Box();
    ~Box();

    void init(b2World* world, const glm::vec2& position, const glm::vec2& dimensions, Bengine::ColorRGBA8 color);

    b2Body* getBody() const { return m_body; }
    b2Fixture* getFixture() const { return m_fixture; }
    const glm::vec2& getDimensions() const { return m_dimensions; }
    const  Bengine::ColorRGBA8& getColor() const { return m_color; }
private:
    b2Body* m_body = nullptr;
    b2Fixture* m_fixture = nullptr;
    glm::vec2 m_dimensions;
    Bengine::ColorRGBA8 m_color;
};

