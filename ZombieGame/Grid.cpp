#include "Grid.h"
#include "Player.h"
#include "Level.h"

#include <iostream>
#include <random>
#include <ctime>
#include <glm/gtx/rotate_vector.hpp>

#include <cmath>

const float HUMAN_SPEED = 1.0f;
const float ZOMBIE_SPEED = 1.3f;

void Grid::init(float levelWidth, float levelHeight, Bengine::ParticleBatch2D* bloodParticleBatch) {
    int numXCells = (int)ceil(levelWidth * TILE_WIDTH / CELL_WIDTH);
    int numYCells = (int)ceil(levelHeight * TILE_WIDTH / CELL_WIDTH);
    m_bloodParticleBatch = bloodParticleBatch;

    // Set up the grid of cells
    m_cells.resize(numYCells);
    for (auto& row : m_cells) {
        row.resize(numXCells);
    }
}

void Grid::draw(Bengine::SpriteBatch& spriteBatch, Bengine::Camera2D& camera) {
    const glm::vec2 agentDims(AGENT_RADIUS * 2.0f);

    for (auto& row : m_cells) {
        for (auto& cell : row) {

            // Draw the humans
            for (int i = 0; i < cell.humans.size(); i++) {
                if (camera.isBoxInView(cell.humans[i]->getPosition(), agentDims)) {
                    cell.humans[i]->draw(spriteBatch);
                }
            }

            // Draw the zombies
            for (int i = 0; i < cell.zombies.size(); i++) {
                if (camera.isBoxInView(cell.zombies[i]->getPosition(), agentDims)) {
                    cell.zombies[i]->draw(spriteBatch);
                }
            }

            // Draw the bullets
            for (int i = 0; i < cell.bullets.size(); i++) {
                cell.bullets[i].draw(spriteBatch);
            }
        }
    }
}

void Grid::addHuman(const glm::vec2& pos) {
    
    // Determine which cell we are in
    Cell& cell = getContainingCell(pos);

    cell.humans.push_back(new Human);
    cell.humans.back()->init(HUMAN_SPEED, pos, this);
}

void Grid::addZombie(const glm::vec2& pos) {
    // Determine which cell we are in
    Cell& cell = getContainingCell(pos);

    cell.zombies.push_back(new Zombie);
    cell.zombies.back()->init(ZOMBIE_SPEED, pos, this);
}

void Grid::addBullet(const glm::vec2& pos, const glm::vec2& direction, float damage, float speed) {
    Cell& cell = getContainingCell(pos);
    cell.bullets.emplace_back(pos, direction, damage, speed, this);
}

bool Grid::update(float deltaTime, const std::vector<std::string>& levelData, Player* player) {

    for (int y = 0; y < m_cells.size(); y++) {
        for (int x = 0; x < m_cells[y].size(); x++) {

            Cell& cell = m_cells[y][x];

            // Update all humans
            for (int i = 0; i < cell.humans.size(); i++) {
                cell.humans[i]->update(levelData,
                                       deltaTime);
                Cell& newCell = getContainingCell(cell.humans[i]->getPosition());
                if (&newCell != &cell) {
                    newCell.humans.push_back(cell.humans[i]);
                    cell.humans[i] = cell.humans.back();
                    cell.humans.pop_back();
                    i--;
                }
            }

            // Update all zombies
            for (int i = 0; i < cell.zombies.size(); i++) {
                cell.zombies[i]->update(levelData,
                                        deltaTime);
                Cell& newCell = getContainingCell(cell.zombies[i]->getPosition());
                if (&newCell != &cell) {
                    newCell.zombies.push_back(cell.zombies[i]);
                    cell.zombies[i] = cell.zombies.back();
                    cell.zombies.pop_back();
                    i--;
                }
            }

            // Update Zombie collisions
            for (int i = 0; i < cell.zombies.size(); i++) {
                updateZombieCollision(cell.zombies[i], i + 1, cell.zombies, cell.humans);
                // Check neighbors
                if (y > 0) updateZombieCollision(cell.zombies[i], 0, m_cells[y - 1][x].zombies, m_cells[y - 1][x].humans);
                if (y > 0 && x > 0) updateZombieCollision(cell.zombies[i], 0, m_cells[y - 1][x - 1].zombies, m_cells[y - 1][x - 1].humans);
                if (x > 0) updateZombieCollision(cell.zombies[i], 0, m_cells[y][x - 1].zombies, m_cells[y][x - 1].humans);
                if (y < m_cells.size() - 1 && x > 0) updateZombieCollision(cell.zombies[i], 0, m_cells[y + 1][x - 1].zombies, m_cells[y + 1][x - 1].humans);
           
                // Collide with player
                if (cell.zombies[i]->collideWithAgent(player)) {
                    return true;
                }
            }

            // Update Human collisions
            for (int i = 0; i < cell.humans.size(); i++) {
                // Collide with other humans
                updateHumanCollision(cell.humans[i], i + 1, cell.humans);
                // Check neighbors
                if (y > 0) updateHumanCollision(cell.humans[i], 0, m_cells[y - 1][x].humans);
                if (y > 0 && x > 0) updateHumanCollision(cell.humans[i], 0, m_cells[y - 1][x - 1].humans);
                if (x > 0) updateHumanCollision(cell.humans[i], 0, m_cells[y][x - 1].humans);
                if (y < m_cells.size() - 1 && x > 0) updateHumanCollision(cell.humans[i], 0, m_cells[y + 1][x - 1].humans);
            }
        }
    }
    return false;
}

void Grid::updateBullets(Cell& cell, int x, int y, float deltaTime, const std::vector<std::string>& levelData) {
    // Update and collide with world
    for (int i = 0; i < cell.bullets.size();) {
        // If update returns true, the bullet collided with a wall
        if (cell.bullets[i].update(levelData, deltaTime)) {
            cell.bullets[i] = cell.bullets.back();
            cell.bullets.pop_back();
        } else {
            Cell& newCell = getContainingCell(cell.bullets[i].getPosition());
            if (&newCell != &cell) {
                newCell.bullets.push_back(cell.bullets[i]);
                cell.bullets[i] = cell.bullets.back();
                cell.bullets.pop_back();
            } else {
                i++;
            }
        }
    }

    // Collide with humans and zombies
    for (int i = 0; i < cell.bullets.size(); i++) {
        if (updateBulletCollision(cell.bullets[i], cell)) {
            i--; // Make sure we don't skip a bullet
            continue;
        }
        if (y > 0 && updateBulletCollision(cell.bullets[i], m_cells[y - 1][x])) {
            continue;
        }
        if (y > 0 && x > 0 && updateBulletCollision(cell.bullets[i], m_cells[y - 1][x - 1])) {
            continue;
        }
        if (x > 0 && updateBulletCollision(cell.bullets[i], m_cells[y][x - 1])) {
            continue;
        }
        if (y < m_cells.size() - 1 && x > 0 && updateBulletCollision(cell.bullets[i], m_cells[y + 1][x - 1])) {
            continue;
        }
    }
}

void Grid::updateZombieCollision(Zombie* zombie, int startZombie,
                                 std::vector<Zombie*>& zombies, std::vector<Human*>& humans) {

    // Collide with other zombies
    for (int j = startZombie; j < zombies.size(); j++) {
        zombie->collideWithAgent(zombies[j]);
    }
    // Collide with humans
    for (int j = 1; j < humans.size(); j++) {
        if (zombie->collideWithAgent(humans[j])) {
            // Add the new zombie
            zombies.push_back(new Zombie);
            zombies.back()->init(ZOMBIE_SPEED, humans[j]->getPosition(), this);
            // Delete the human
            delete humans[j];
            humans[j] = humans.back();
            humans.pop_back();
        }
    } 
}

void Grid::updateHumanCollision(Human* human, int startHuman, std::vector<Human*>& humans) {
    for (int j = startHuman; j < humans.size(); j++) {
        human->collideWithAgent(humans[j]);
    }
}

bool Grid::updateBulletCollision(Bullet& bullet, Cell& cell) {
    bool wasBulletRemoved = false;
    // Loop through zombies
    for (int j = 0; j < cell.zombies.size();) {
        // Check collision
        if (bullet.collideWithAgent(cell.zombies[j])) {
            // Add blood
            addBlood(bullet.getPosition(), 5);

            // Damage zombie, and kill it if its out of health
            if (cell.zombies[j]->applyDamage(bullet.getDamage())) {
                // If the zombie died, remove him
                delete cell.zombies[j];
                cell.zombies[j] = cell.zombies.back();
                cell.zombies.pop_back();
                m_numZombiesKilled++;
            } else {
                j++;
            }

            // Remove the bullet
            bullet = cell.bullets.back();
            cell.bullets.pop_back();
            // Since the bullet died, no need to loop through any more zombies
            return true;
        } else {
            j++;
        }
    }
    // Loop through humans

    for (int j = 1; j < cell.humans.size();) {
        // Check collision
        if (bullet.collideWithAgent(cell.humans[j])) {
            // Add blood
            addBlood(bullet.getPosition(), 5);
            // Damage human, and kill it if its out of health
            if (cell.humans[j]->applyDamage(bullet.getDamage())) {
                // If the human died, remove him
                delete cell.humans[j];
                cell.humans[j] = cell.humans.back();
                cell.humans.pop_back();
            } else {
                j++;
            }

            // Remove the bullet
            bullet = cell.bullets.back();
            cell.bullets.pop_back();
            m_numHumansKilled++;
            return true;
            // Since the bullet died, no need to loop through any more zombies
            break;
        } else {
            j++;
        }
    }
    return false;
}

Cell& Grid::getContainingCell(const glm::vec2& pos) {
    // Determine which cell we are in
    int cellX = (int)(pos.x / CELL_WIDTH);
    int cellY = (int)(pos.y / CELL_WIDTH);
    // Clamp position
    if (cellX < 0) { cellX = 0; } else if (cellX >= m_cells[cellY].size()) { cellX = m_cells[cellY].size() - 1; }
    if (cellY < 0) { cellY = 0; } else if (cellY >= m_cells.size()) { cellY = m_cells.size() - 1; }
    // Get the cell
    return m_cells[cellY][cellX];
}

void Grid::destroy() {
    for (auto& row : m_cells) {
        for (auto& cell : row) {
            // Don't forget to delete the humans and zombies!
            for (int i = 0; i < cell.humans.size(); i++) {
                delete cell.humans[i];
            }
            for (int i = 0; i < cell.zombies.size(); i++) {
                delete cell.zombies[i];
            }
        }
    }

    m_cells.clear();
}

void Grid::addBlood(const glm::vec2& position, int numParticles) {

    static std::mt19937 randEngine(time(nullptr));
    static std::uniform_real_distribution<float> randAngle(0.0f, 360.0f);

    glm::vec2 vel(2.0f, 0.0f);
    Bengine::ColorRGBA8 col(255, 0, 0, 255);

    for (int i = 0; i < numParticles; i++) {
        m_bloodParticleBatch->addParticle(position, glm::rotate(vel, randAngle(randEngine)), col, 30.0f);
    }
}