#pragma once

#include <Bengine/IMainGame.h>
#include "GameplayScreen.h"
#include "MainMenuScreen.h"

// Our custom app that inherits from IMainGame
class App : public Bengine::IMainGame{
public:
    App();
    ~App();

    // Called on initialization
    virtual void onInit() override;
    // For adding all screens
    virtual void addScreens() override;
    // Called when exiting
    virtual void onExit() override;
private:
    std::unique_ptr<GameplayScreen> m_gameplayScreen = nullptr;
    std::unique_ptr<MainMenuScreen> m_mainMenuScreen = nullptr;
};

