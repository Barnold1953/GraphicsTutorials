#pragma once

#include "Human.h"
#include "Zombie.h"
#include "Bullet.h"

#include <Bengine/ParticleBatch2D.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/Camera2D.h>

class Player;

#include <vector>

struct Cell {
    std::vector<Human*> humans;
    std::vector<Zombie*> zombies;
    std::vector<Bullet> bullets;
};

class Grid {
public:
    void init(float levelWidth, float levelHeight, Bengine::ParticleBatch2D* bloodParticleBatch);

    void draw(Bengine::SpriteBatch& spriteBatch, Bengine::Camera2D& camera);

    void addHuman(const glm::vec2& pos);
    void addZombie(const glm::vec2& pos);
    void addBullet(const glm::vec2& pos, const glm::vec2& direction, float damage, float speed);

    //Returns true when the game ends
    bool update(float deltaTime, const std::vector<std::string>& levelData, Player* player);

    Cell& getContainingCell(const glm::vec2& pos);

    void destroy();

    const int CELL_WIDTH = 512;

    int getNumZombiesKilled() const { return m_numZombiesKilled; }
    int getNumHumansKilled() const { return m_numHumansKilled; }
private:

    void updateBullets(Cell& cell, int x, int y, float deltaTime, const std::vector<std::string>& levelData);

    void updateZombieCollision(Zombie* zombie, int startZombie,
                               std::vector<Zombie*>& zombies, std::vector<Human*>& humans);

    void updateHumanCollision(Human* human, int startHuman, std::vector<Human*>& humans);

    bool updateBulletCollision(Bullet& bullet, Cell& cell);

    /// Adds blood to the particle engine
    void addBlood(const glm::vec2& position, int numParticles);

    Bengine::ParticleBatch2D* m_bloodParticleBatch = nullptr;
    std::vector< std::vector<Cell> > m_cells;
    int m_numHumansKilled = 0;
    int m_numZombiesKilled = 0;
};

