#include "Gun.h"
#include <random>
#include <ctime>
#include <glm/gtx/rotate_vector.hpp>


Gun::Gun(std::string name, int fireRate, int bulletsPerShot,
         float spread, float bulletDamage, float bulletSpeed, Bengine::SoundEffect fireEffect) :
    _name(name),
    _fireRate(fireRate),
    _bulletsPerShot(bulletsPerShot),
    _spread(spread),
    _bulletDamage(bulletDamage),
    _bulletSpeed(bulletSpeed), 
    _frameCounter(0),
    m_fireEffect(fireEffect) {
    // Empty
}


Gun::~Gun() {
    // Empty
}

void Gun::update(bool isMouseDown, const glm::vec2& position, const glm::vec2& direction, std::vector<Bullet>& bullets, float deltaTime) {
    _frameCounter += 1.0f * deltaTime;
    // After a certain number of frames has passed we fire our gun
    if (_frameCounter >= _fireRate && isMouseDown) {
        fire(direction, position, bullets);
        _frameCounter = 0;
    }
}

void Gun::fire(const glm::vec2& direction, const glm::vec2& position, std::vector<Bullet>& bullets) {

    static std::mt19937 randomEngine(time(nullptr));
    // For offsetting the accuracy
    std::uniform_real_distribution<float> randRotate(-_spread, _spread);

    m_fireEffect.play();

    for (int i = 0; i < _bulletsPerShot; i++) {
        // Add a new bullet
        bullets.emplace_back(position - glm::vec2(BULLET_RADIUS), 
                             glm::rotate(direction, randRotate(randomEngine)),
                             _bulletDamage, 
                             _bulletSpeed);
    }   

}
