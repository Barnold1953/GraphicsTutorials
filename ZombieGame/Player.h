#pragma once

#include "Human.h"
#include <Bengine/InputManager.h>
#include <Bengine/Camera2D.h>
#include "Bullet.h"

class Gun;

class Player : public Human
{
public:
    Player();
    ~Player();

    void init(float speed, glm::vec2 pos, Bengine::InputManager* inputManager, Bengine::Camera2D* camera, Grid* grid);

    void addGun(Gun* gun);

    void update(const std::vector<std::string>& levelData,
                float deltaTime) override;
private:
    Bengine::InputManager* _inputManager;

    std::vector<Gun*> _guns;
    int _currentGunIndex;

    Bengine::Camera2D* _camera;

};

