#pragma once

#include "Agent.h"

class Zombie : public Agent
{
public:
    Zombie();
    ~Zombie();

    void init(float speed, glm::vec2 pos, Grid* grid);

    virtual void update(const std::vector<std::string>& levelData,
                        float deltaTime) override;
private:

    Human* getNearestHuman(std::vector<Human*>& humans);
};

