#pragma once

#include "Player.h"
#include <Bengine/IGameScreen.h>
#include <vector>
#include <Bengine/SpriteBatch.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/Camera2D.h>
#include <Bengine/GLTexture.h>
#include <Bengine/Window.h>
#include <Bengine/DebugRenderer.h>

#include <Bengine/GUI.h>

#include "ScreenIndices.h"

class MainMenuScreen : public Bengine::IGameScreen {
public:
    MainMenuScreen(Bengine::Window* window);
    ~MainMenuScreen();

    virtual int getNextScreenIndex() const override;

    virtual int getPreviousScreenIndex() const override;

    virtual void build() override;

    virtual void destroy() override;

    virtual void onEntry() override;

    virtual void onExit() override;

    virtual void update() override;

    virtual void draw() override;

private:
    void initUI();
    void checkInput();

    bool onNewGameClicked(const CEGUI::EventArgs& e);
    bool onEditorClicked(const CEGUI::EventArgs& e);
    bool onExitClicked(const CEGUI::EventArgs& e);

    int m_nextScreenIndex = SCREEN_INDEX_GAMEPLAY;
    Bengine::Camera2D m_camera;
    Bengine::Window* m_window;
    Bengine::GUI m_gui;
};