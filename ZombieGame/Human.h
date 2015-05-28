#pragma once

#include "Agent.h"

class Human : public Agent
{
public:
    Human();
    virtual ~Human();

    void init(float speed, glm::vec2 pos, Grid* grid);

    virtual void update(const std::vector<std::string>& levelData,
                        float deltaTime) override;

private:
    int _frames;
};

