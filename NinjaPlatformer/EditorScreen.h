#pragma once

#include "Box.h"
#include "Light.h"
#include "Player.h"
#include <Bengine/Camera2D.h>
#include <Bengine/DebugRenderer.h>
#include <Bengine/GLSLProgram.h>
#include <Bengine/GLTexture.h>
#include <Bengine/IGameScreen.h>
#include <Bengine/InputManager.h>
#include <Bengine/SpriteBatch.h>
#include <Bengine/SpriteFont.h>
#include <Bengine/Window.h>
#include <vector>

#include <Bengine/GUI.h>

#include "ScreenIndices.h"

enum class PhysicsMode {
    RIGID,
    DYNAMIC
};

enum class ObjectMode {
    PLAYER,
    PLATFORM,
    LIGHT,
    FINISH
};

enum class SelectionMode {
    SELECT,
    PLACE
};

const int NO_BOX = -1;
const int NO_LIGHT = -1;

class WidgetLabel {
public:
    WidgetLabel() {};
    WidgetLabel(CEGUI::Window* w, const std::string& text, float scale) :
        widget(w), text(text), scale(scale) {
        // Empty
    };
    void draw(Bengine::SpriteBatch& sb, Bengine::SpriteFont& sf, Bengine::Window* w);

    CEGUI::Window* widget = nullptr;
    std::string text = "";
    Bengine::ColorRGBA8 color = Bengine::ColorRGBA8(255, 255, 255, 255);
    float scale = 0.7f;
};

class EditorScreen : public Bengine::IGameScreen {
public:
    EditorScreen(Bengine::Window* window);
    ~EditorScreen();

    /************************************************************************/
    /* IGameScreen Implementation                                           */
    /************************************************************************/
    virtual int getNextScreenIndex() const override;
    virtual int getPreviousScreenIndex() const override;
    virtual void build() override;
    virtual void destroy() override;
    virtual void onEntry() override;
    virtual void onExit() override;
    virtual void update() override;
    virtual void draw() override;

private:
    void drawUI();
    void drawWorld();

    void initUI();
    void checkInput();
    void updateMouseDown(const SDL_Event& evnt);
    void updateMouseUp(const SDL_Event& evnt);
    void updateMouseMotion(const SDL_Event& evnt);
    void refreshSelectedBox();
    void refreshSelectedBox(const glm::vec2& newPosition);
    void refreshSelectedLight();
    void refreshSelectedLight(const glm::vec2& newPosition);
    bool isMouseInUI();

    void setPlatformWidgetVisibility(bool visible);
    void setLightWidgetVisibility(bool visible);

    /************************************************************************/
    /* Event Handlers                                                       */
    /************************************************************************/
    bool onExitClicked(const CEGUI::EventArgs& e);
    bool onColorPickerRedChanged(const CEGUI::EventArgs& e);
    bool onColorPickerGreenChanged(const CEGUI::EventArgs& e);
    bool onColorPickerBlueChanged(const CEGUI::EventArgs& e);
    bool onColorPickerAlphaChanged(const CEGUI::EventArgs& e);
    bool onRigidMouseClick(const CEGUI::EventArgs& e);
    bool onDynamicMouseClick(const CEGUI::EventArgs& e);
    bool onPlayerMouseClick(const CEGUI::EventArgs& e);
    bool onPlatformMouseClick(const CEGUI::EventArgs& e);
    bool onFinishMouseClick(const CEGUI::EventArgs& e);
    bool onLightMouseClick(const CEGUI::EventArgs& e);
    bool onSelectMouseClick(const CEGUI::EventArgs& e);
    bool onPlaceMouseClick(const CEGUI::EventArgs& e);
    bool onSaveMouseClick(const CEGUI::EventArgs& e);
    bool onBackMouseClick(const CEGUI::EventArgs& e);
    bool onRotationValueChange(const CEGUI::EventArgs& e);
    bool onSizeValueChange(const CEGUI::EventArgs& e);
    bool onWidthValueChange(const CEGUI::EventArgs& e);
    bool onHeightValueChange(const CEGUI::EventArgs& e);
    bool onDebugToggleClick(const CEGUI::EventArgs& e);
    
    /************************************************************************/
    /* Member Variables                                                     */
    /************************************************************************/
    float m_colorPickerRed = 255.0f;
    float m_colorPickerGreen = 255.0f;
    float m_colorPickerBlue = 255.0f;
    float m_colorPickerAlpha = 255.0f;
    float m_rotation = 0.0f;
    float m_lightSize = 10.0f;
    glm::vec2 m_boxDims = glm::vec2(1.0f);
    bool m_mouseButtons[2];
    bool m_debugRender = false;

    CEGUI::GroupBox* m_groupBox = nullptr;
    CEGUI::Slider* m_rSlider = nullptr;
    CEGUI::Slider* m_gSlider = nullptr;
    CEGUI::Slider* m_bSlider = nullptr;
    CEGUI::Slider* m_aSlider = nullptr;
    CEGUI::RadioButton* m_rigidRadioButton = nullptr;
    CEGUI::RadioButton* m_dynamicRadioButton = nullptr;
    CEGUI::RadioButton* m_playerRadioButton = nullptr;
    CEGUI::RadioButton* m_platformRadioButton = nullptr;
    CEGUI::RadioButton* m_finishRadioButton = nullptr;
    CEGUI::RadioButton* m_lightRadioButton = nullptr;
    CEGUI::RadioButton* m_selectRadioButton = nullptr;
    CEGUI::RadioButton* m_placeRadioButton = nullptr;
    CEGUI::ToggleButton* m_debugToggle = nullptr;
    CEGUI::Spinner* m_rotationSpinner = nullptr;
    CEGUI::Spinner* m_widthSpinner = nullptr;
    CEGUI::Spinner* m_heightSpinner = nullptr;
    CEGUI::Spinner* m_sizeSpinner = nullptr;
    std::vector<WidgetLabel> m_widgetLabels;

    Bengine::SpriteBatch m_spriteBatch;
    Bengine::SpriteFont m_spriteFont;
    Bengine::GLSLProgram m_textureProgram;
    Bengine::GLSLProgram m_lightProgram;
    Bengine::DebugRenderer m_debugRenderer;

    Bengine::GLTexture m_blankTexture;

    PhysicsMode m_physicsMode = PhysicsMode::RIGID;
    ObjectMode m_objectMode = ObjectMode::PLAYER;
    SelectionMode m_selectMode = SelectionMode::SELECT;

    Bengine::InputManager m_inputManager;

    bool m_hasPlayer = false;
    Player m_player;
    std::vector<Box> m_boxes;
    std::vector<Light> m_lights;
    int m_selectedBox = NO_BOX;
    int m_selectedLight = NO_LIGHT;
    bool m_isDragging = false;
    glm::vec2 m_selectOffset;

    Bengine::Camera2D m_camera;
    Bengine::Camera2D m_uiCamera;
    Bengine::Window* m_window;
    Bengine::GUI m_gui;
    std::unique_ptr<b2World> m_world;
};