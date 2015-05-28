#include "MainGame.h"

#include <Bengine/Bengine.h>
#include <Bengine/Timing.h>
#include <Bengine/BengineErrors.h>
#include <Bengine/ResourceManager.h>
#include <random>
#include <ctime>
#include <algorithm>

#include <SDL/SDL.h>
#include <iostream>
#include <glm/gtx/rotate_vector.hpp>

#include "Gun.h"
#include "Zombie.h"

const float PLAYER_SPEED = 5.0f;

MainGame::MainGame()  :
    m_screenWidth(1600),
    m_screenHeight(900),
    m_gameState(GameState::PLAY),
    m_fps(0),
    m_player(nullptr),
    m_numHumansKilled(0),
    m_numZombiesKilled(0) {
    // Empty
    
}

MainGame::~MainGame() {
    // Don't forget to delete the levels!
    for (int i = 0; i < m_levels.size(); i++) {
        delete m_levels[i];
    }

    m_grid.destroy();
}

void MainGame::run() {

    initSystems();

    initLevel();

    Bengine::Music music = m_audioEngine.loadMusic("Sound/XYZ.ogg");
    music.play(-1);

    gameLoop();
}

void MainGame::initSystems() {
    // Initialize the game engine
    Bengine::init();

    // Initialize sound, must happen after Bengine::init
    m_audioEngine.init();

    // Create our window
    m_window.create("ZombieGame", m_screenWidth, m_screenHeight, 0);

    // Grey background color
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);

    // Set up the shaders
    initShaders();

    // Initialize our spritebatch
    m_agentSpriteBatch.init();
    m_hudSpriteBatch.init();

    // Initialize sprite font
    m_spriteFont = new Bengine::SpriteFont("Fonts/chintzy.ttf", 64);

    // Set up the camera
    m_camera.init(m_screenWidth, m_screenHeight);
    m_hudCamera.init(m_screenWidth, m_screenHeight);
    m_hudCamera.setPosition(glm::vec2(m_screenWidth / 2, m_screenHeight / 2));

    // Initialize particles
    m_bloodParticleBatch = new Bengine::ParticleBatch2D;

    // Initialize the particle batch and use a lambda function to define the update
    m_bloodParticleBatch->init(1000, 0.05f,
                               Bengine::ResourceManager::getTexture("Textures/particle.png"),
                               [](Bengine::Particle2D& particle, float deltaTime) {
        particle.position += particle.velocity * deltaTime;
        particle.color.a = (GLubyte)(particle.life * 255.0f);
    });

    m_particleEngine.addParticleBatch(m_bloodParticleBatch);

}

void MainGame::initLevel() {
    // Level 1
    m_levels.push_back(new Level("Levels/level1.txt"));
    m_currentLevel = 0;

    m_player = new Player();
    m_player->init(PLAYER_SPEED, m_levels[m_currentLevel]->getStartPlayerPos(), &m_inputManager, &m_camera, &m_grid);

    m_grid.init(m_levels[m_currentLevel]->getWidth(), m_levels[m_currentLevel]->getHeight(), m_bloodParticleBatch);

    std::mt19937 randomEngine;
    randomEngine.seed(time(nullptr));

    std::uniform_int_distribution<int> randX(2, m_levels[m_currentLevel]->getWidth() - 2);
    std::uniform_int_distribution<int> randY(2, m_levels[m_currentLevel]->getHeight() - 2);

    // Add all the random humans
    m_totalHumans = m_levels[m_currentLevel]->getNumHumans();
    for (int i = 0; i < m_levels[m_currentLevel]->getNumHumans(); i++) {
        m_grid.addHuman(glm::vec2(randX(randomEngine) * TILE_WIDTH, randY(randomEngine) * TILE_WIDTH));
    }

    // Add the zombies
    const std::vector<glm::vec2>& zombiePositions = m_levels[m_currentLevel]->getZombieStartPositions();
    m_totalZombies = m_levels[m_currentLevel]->getZombieStartPositions().size();
    for (int i = 0; i < zombiePositions.size(); i++) {
        m_grid.addZombie(zombiePositions[i]);
    }

    // Set up the players guns
    const float BULLET_SPEED = 20.0f;
    m_player->addGun(new Gun("Magnum", 10, 1, 5.0f, 30, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/shots/pistol.wav")));
    m_player->addGun(new Gun("Shotgun", 30, 12, 20.0f, 4, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/shots/shotgun.wav")));
    m_player->addGun(new Gun("MP5", 2, 1, 10.0f, 20, BULLET_SPEED, m_audioEngine.loadSoundEffect("Sound/shots/cg1.wav")));
}

void MainGame::initShaders() {
    // Compile our color shader
    m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
    m_textureProgram.addAttribute("vertexPosition");
    m_textureProgram.addAttribute("vertexColor");
    m_textureProgram.addAttribute("vertexUV");
    m_textureProgram.linkShaders();
}

void MainGame::gameLoop() {
    
    // Some helpful constants.
    const float DESIRED_FPS = 60.0f; // FPS the game is designed to run at
    const int MAX_PHYSICS_STEPS = 6; // Max number of physics steps per frame
    const float MS_PER_SECOND = 1000; // Number of milliseconds in a second
    const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIRED_FPS; // The desired frame time per frame
    const float MAX_DELTA_TIME = 1.0f; // Maximum size of deltaTime

    // Used to cap the FPS
    Bengine::FpsLimiter fpsLimiter;
    fpsLimiter.setMaxFPS(60000.0f);

    // Zoom out the camera by 3x
    const float CAMERA_SCALE = 1.0f / 8.0f;
    m_camera.setScale(CAMERA_SCALE);

    // Start our previousTicks variable
    float previousTicks = SDL_GetTicks();

    // Main loop
    while (m_gameState == GameState::PLAY) {
        fpsLimiter.begin();

        // Calculate the frameTime in milliseconds
        float newTicks = SDL_GetTicks();
        float frameTime = newTicks - previousTicks;
        previousTicks = newTicks; // Store newTicks in previousTicks so we can use it next frame
        // Get the total delta time
        float totalDeltaTime = frameTime / DESIRED_FRAMETIME;

        checkVictory();

        m_inputManager.update();

        processInput();
        
        int i = 0; // This counter makes sure we don't spiral to death!
        // Loop while we still have steps to process.
        while (totalDeltaTime > 0.0f && i < MAX_PHYSICS_STEPS) {
            // The deltaTime should be the the smaller of the totalDeltaTime and MAX_DELTA_TIME
            float deltaTime = std::min(totalDeltaTime, MAX_DELTA_TIME);
            // Update all physics here and pass in deltaTime
            
            if (m_grid.update(deltaTime, m_levels[m_currentLevel]->getLevelData(), m_player)) {
                Bengine::fatalError("YOU LOSE");
            }

            m_particleEngine.update(deltaTime);
            // Since we just took a step that is length deltaTime, subtract from totalDeltaTime
            totalDeltaTime -= deltaTime;
            // Increment our frame counter so we can limit steps to MAX_PHYSICS_STEPS
            i++;
        }

        // Make sure the camera is bound to the player position
        m_camera.setPosition(m_player->getPosition());
        m_camera.update();
        m_hudCamera.update();

        drawGame();

        // End the frame, limit the FPS, and get the current FPS.
        m_fps = fpsLimiter.end();
        std::cout << m_fps << std::endl;
    }
}

void MainGame::checkVictory() {
    // TODO: Support for multiple levels!
    // _currentLevel++; initLevel(...);

    // If all zombies are dead we win!
    if (m_grid.getNumZombiesKilled() == m_totalZombies) {
        // Print victory message
        std::printf("*** You win! ***\n You killed %d humans and %d zombies. There are %d/%d civilians remaining",
                    m_numHumansKilled, m_numZombiesKilled, m_totalHumans, m_levels[m_currentLevel]->getNumHumans());

        // Easy way to end the game :P
        Bengine::fatalError("");
    }
}

void MainGame::processInput() {
    SDL_Event evnt;
    //Will keep looping until there are no more events to process
    while (SDL_PollEvent(&evnt)) {
        switch (evnt.type) {
            case SDL_QUIT:
                m_gameState = GameState::EXIT;
                break;
            case SDL_MOUSEMOTION:
                m_inputManager.setMouseCoords(evnt.motion.x, evnt.motion.y);
                break;
            case SDL_KEYDOWN:
                m_inputManager.pressKey(evnt.key.keysym.sym);
                break;
            case SDL_KEYUP:
                m_inputManager.releaseKey(evnt.key.keysym.sym);
                break;
            case SDL_MOUSEBUTTONDOWN:
                m_inputManager.pressKey(evnt.button.button);
                break;
            case SDL_MOUSEBUTTONUP:
                m_inputManager.releaseKey(evnt.button.button);
                break;
        }
    }
}

void MainGame::drawGame() {
    // Set the base depth to 1.0
    glClearDepth(1.0);
    // Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_textureProgram.use();

    // Draw code goes here
    glActiveTexture(GL_TEXTURE0);

    // Make sure the shader uses texture 0
    GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);

    // Grab the camera matrix
    glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
    GLint pUniform = m_textureProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    // Draw the level
    m_levels[m_currentLevel]->draw();

    // Begin drawing agents
    m_agentSpriteBatch.begin();

    m_grid.draw(m_agentSpriteBatch, m_camera);

    // End spritebatch creation
    m_agentSpriteBatch.end();

    // Render to the screen
    m_agentSpriteBatch.renderBatch();

    // Render the particles
    m_particleEngine.draw(&m_agentSpriteBatch);

    // Render the heads up display
    drawHud();

    // Unbind the program
    m_textureProgram.unuse();

    // Swap our buffer and draw everything to the screen!
    m_window.swapBuffer();
}

void MainGame::drawHud() {
    char buffer[256];
    
    glm::mat4 projectionMatrix = m_hudCamera.getCameraMatrix();
    GLint pUniform = m_textureProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    m_hudSpriteBatch.begin();

    sprintf_s(buffer, "Num Humans %d", m_totalHumans - m_grid.getNumHumansKilled());
    m_spriteFont->draw(m_hudSpriteBatch, buffer, glm::vec2(0, 0),
                      glm::vec2(0.5), 0.0f, Bengine::ColorRGBA8(255, 255, 255, 255));

    sprintf_s(buffer, "Num Zombies %d", m_totalZombies - m_grid.getNumZombiesKilled());
    m_spriteFont->draw(m_hudSpriteBatch, buffer, glm::vec2(0, 36),
                      glm::vec2(0.5), 0.0f, Bengine::ColorRGBA8(255, 255, 255, 255));

    m_hudSpriteBatch.end();
    m_hudSpriteBatch.renderBatch();
}