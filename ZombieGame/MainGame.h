#pragma once

#include <Bengine/Window.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/Camera2D.h>
#include <Bengine/InputManager.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/SpriteFont.h>
#include <Bengine/AudioEngine.h>
#include <Bengine/ParticleEngine2D.h>
#include <Bengine/ParticleBatch2D.h>

#include "Player.h"
#include "Level.h"
#include "Bullet.h"
#include "Grid.h"

class Zombie;

enum class GameState {
    PLAY,
    EXIT
};

class MainGame
{
public:
    MainGame();
    ~MainGame();

    /// Runs the game
    void run();

private:
    /// Initializes the core systems
    void initSystems();

    /// Initializes the level and sets up everything
    void initLevel();

    /// Initializes the shaders
    void initShaders();

    /// Main game loop for the program
    void gameLoop();

    /// Checks the victory condition
    void checkVictory();

    /// Handles input processing
    void processInput();

    /// Renders the game
    void drawGame();

    /// Draws the HUD
    void drawHud();

    /// Member Variables
    Bengine::Window m_window; ///< The game window
    
    Bengine::GLSLProgram m_textureProgram; ///< The shader program

    Bengine::InputManager m_inputManager; ///< Handles input

    Bengine::Camera2D m_camera; ///< Main Camera
    Bengine::Camera2D m_hudCamera; ///< Hud Camera

    Bengine::SpriteBatch m_agentSpriteBatch; ///< Draws all agents
    Bengine::SpriteBatch m_hudSpriteBatch;

    Bengine::ParticleEngine2D m_particleEngine;
    Bengine::ParticleBatch2D* m_bloodParticleBatch;

    std::vector<Level*> m_levels; ///< vector of all levels

    int m_screenWidth = 1024;
    int m_screenHeight = 768;

    float m_fps;

    int m_currentLevel;

    Player* m_player;
    Grid m_grid;

    int m_numHumansKilled; ///< Humans killed by player
    int m_numZombiesKilled; ///< Zombies killed by player

    Bengine::SpriteFont* m_spriteFont;

    Bengine::AudioEngine m_audioEngine;

    int m_totalZombies = 0;
    int m_totalHumans = 0;

    GameState m_gameState;
};

