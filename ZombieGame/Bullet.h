#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <Bengine/SpriteBatch.h>

class Human;
class Zombie;
class Agent;
class Grid;

const int BULLET_RADIUS = 5;

class Bullet
{
public:
    Bullet(glm::vec2 position, glm::vec2 direction, float damage, float speed, Grid* grid);
    ~Bullet();

    // When update returns true, delete bullet
    bool update(const std::vector<std::string>& levelData, float deltaTime);

    void draw(Bengine::SpriteBatch& spriteBatch);

    bool collideWithAgent(Agent* agent);

    float getDamage() const { return _damage; }

    glm::vec2 getPosition() const { return _position; }

private:

    bool collideWithWorld(const std::vector<std::string>& levelData);

    glm::vec2 _position;
    glm::vec2 _direction;
    float _damage;
    float _speed;
    Grid* m_grid = nullptr;
};

