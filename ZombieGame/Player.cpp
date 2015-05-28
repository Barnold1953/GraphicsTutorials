#include "Player.h"
#include <SDL/SDL.h>
#include <Bengine/ResourceManager.h>

#include "Gun.h"

Player::Player() :
    _currentGunIndex(-1) {
    // Empty
}

Player::~Player() {
    // Empty
}

void Player::init(float speed, glm::vec2 pos, Bengine::InputManager* inputManager, Bengine::Camera2D* camera,
                  Grid* grid) {
    _speed = speed;
    _position = pos;
    _inputManager = inputManager;
    m_grid = grid;
    _camera = camera;
    _color.r = 255;
    _color.g = 255;
    _color.b = 255;
    _color.a = 255;
    _health = 150;
    m_textureID = Bengine::ResourceManager::getTexture("Textures/player.png").id;
}

void Player::addGun(Gun* gun) {
    // Add the gun to his inventory
    _guns.push_back(gun);

    // If no gun equipped, equip gun.
    if (_currentGunIndex == -1) {
        _currentGunIndex = 0;
    }
}

void Player::update(const std::vector<std::string>& levelData,
                    float deltaTime) {

    if (_inputManager->isKeyDown(SDLK_w)) {
        _position.y += _speed * deltaTime;
    } else if (_inputManager->isKeyDown(SDLK_s)) {
        _position.y -= _speed * deltaTime;
    }
    if (_inputManager->isKeyDown(SDLK_a)) {
        _position.x -= _speed * deltaTime;
    } else if (_inputManager->isKeyDown(SDLK_d)) {
        _position.x += _speed * deltaTime;
    }

    if (_inputManager->isKeyDown(SDLK_1) && _guns.size() >= 0) {
        _currentGunIndex = 0;
    } else if (_inputManager->isKeyDown(SDLK_2) && _guns.size() >= 1) {
        _currentGunIndex = 1;
    } else if (_inputManager->isKeyDown(SDLK_3) && _guns.size() >= 2) {
        _currentGunIndex = 2;
    }

    glm::vec2 mouseCoords = _inputManager->getMouseCoords();
    mouseCoords = _camera->convertScreenToWorld(mouseCoords);


    glm::vec2 centerPosition = _position + glm::vec2(AGENT_RADIUS);

    m_direction = glm::normalize(mouseCoords - centerPosition);

    if (_currentGunIndex != -1) {

        _guns[_currentGunIndex]->update(_inputManager->isKeyDown(SDL_BUTTON_LEFT),
                                        centerPosition,
                                        m_direction,
                                        m_grid,
                                        deltaTime);
                                        

    }

    collideWithLevel(levelData);
}
