#include "EditorScreen.h"

#include <Bengine/ResourceManager.h>

const int MOUSE_LEFT = 0;
const int MOUSE_RIGHT = 1;
const float LIGHT_SELECT_RADIUS = 0.5f;

void WidgetLabel::draw(Bengine::SpriteBatch& sb, Bengine::SpriteFont& sf, Bengine::Window* w) {
    if (!widget->isVisible()) return;
    glm::vec2 pos;
    pos.x = widget->getXPosition().d_scale * w->getScreenWidth() - w->getScreenWidth() / 2.0f +
        widget->getWidth().d_offset / 2.0f;
    pos.y = w->getScreenHeight() / 2.0f - widget->getYPosition().d_scale * w->getScreenHeight();
    sf.draw(sb, text.c_str(), pos, glm::vec2(scale), 0.0f, color, Bengine::Justification::MIDDLE);
}

EditorScreen::EditorScreen(Bengine::Window* window) :
m_window(window),
m_spriteFont("Fonts/chintzy.ttf", 32) {
    m_screenIndex = SCREEN_INDEX_EDITOR;
}

EditorScreen::~EditorScreen() {
}

int EditorScreen::getNextScreenIndex() const {
    return SCREEN_INDEX_NO_SCREEN;
}

int EditorScreen::getPreviousScreenIndex() const {
    return SCREEN_INDEX_MAINMENU;
}

void EditorScreen::build() {

}

void EditorScreen::destroy() {

}

void EditorScreen::onEntry() {
    m_mouseButtons[MOUSE_LEFT] = false;
    m_mouseButtons[MOUSE_RIGHT] = false;
    // Init camera
    m_camera.init(m_window->getScreenWidth(), m_window->getScreenHeight());
    m_camera.setScale(32.0f);
    // UI camera has scale 1.0f
    m_uiCamera.init(m_window->getScreenWidth(), m_window->getScreenHeight());
    m_uiCamera.setScale(1.0f);

    m_debugRenderer.init();

    initUI();
    m_spriteBatch.init();

    // Set up box2D stuff
    b2Vec2 gravity(0.0f, -25.0);
    m_world = std::make_unique<b2World>(gravity);

    // Shader init
    // Compile our texture shader
    m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
    m_textureProgram.addAttribute("vertexPosition");
    m_textureProgram.addAttribute("vertexColor");
    m_textureProgram.addAttribute("vertexUV");
    m_textureProgram.linkShaders();
    // Compile our light shader
    m_lightProgram.compileShaders("Shaders/lightShading.vert", "Shaders/lightShading.frag");
    m_lightProgram.addAttribute("vertexPosition");
    m_lightProgram.addAttribute("vertexColor");
    m_lightProgram.addAttribute("vertexUV");
    m_lightProgram.linkShaders();

    m_blankTexture = Bengine::ResourceManager::getTexture("Assets/blank.png");
}

void EditorScreen::onExit() {
    m_gui.destroy();
    m_textureProgram.dispose();
    m_spriteFont.dispose();
    m_widgetLabels.clear();
    m_world.reset();
}

void EditorScreen::update() {
    m_camera.update();
    m_uiCamera.update();
  
    checkInput();

    // Platform scaling and rotation from keypress
    if ((m_objectMode == ObjectMode::PLATFORM && m_selectMode == SelectionMode::PLACE) || m_selectedBox != NO_BOX) {
        const double SCALE_SPEED = 0.01;
        const double ROT_SPEED = 0.01;
        // The callbacks will set the member variables for m_boxDims, don't need to set here
        // Scale
        if (m_inputManager.isKeyDown(SDLK_LEFT)) {
            m_widthSpinner->setCurrentValue(m_widthSpinner->getCurrentValue() - SCALE_SPEED);
        } else if (m_inputManager.isKeyDown(SDLK_RIGHT)) {
            m_widthSpinner->setCurrentValue(m_widthSpinner->getCurrentValue() + SCALE_SPEED);
        }
        if (m_inputManager.isKeyDown(SDLK_DOWN)) {
            m_heightSpinner->setCurrentValue(m_heightSpinner->getCurrentValue() - SCALE_SPEED);
        } else if (m_inputManager.isKeyDown(SDLK_UP)) {
            m_heightSpinner->setCurrentValue(m_heightSpinner->getCurrentValue() + SCALE_SPEED);
        }
        // Rotation
        if (m_inputManager.isKeyDown(SDLK_e)) {
            // Have to check wraparound
            double newValue = m_rotationSpinner->getCurrentValue() - ROT_SPEED;
            if (newValue < 0.0) newValue += M_PI * 2.0;
            m_rotationSpinner->setCurrentValue(newValue);
        } else if (m_inputManager.isKeyDown(SDLK_q)) {
            double newValue = m_rotationSpinner->getCurrentValue() + ROT_SPEED;
            if (newValue > M_PI * 2.0) newValue -= M_PI * 2.0;
            m_rotationSpinner->setCurrentValue(newValue);
        }
    }

    // Light scaling from keypress
    if ((m_objectMode == ObjectMode::LIGHT && m_selectMode == SelectionMode::PLACE) || m_selectedLight != NO_LIGHT) {
        const double SCALE_SPEED = 0.1;
        const float ALPHA_SPEED = 1.0f;
        // Scaling
        if (m_inputManager.isKeyDown(SDLK_LEFT)) {
            m_sizeSpinner->setCurrentValue(m_sizeSpinner->getCurrentValue() - SCALE_SPEED);
        } else if (m_inputManager.isKeyDown(SDLK_RIGHT)) {
            m_sizeSpinner->setCurrentValue(m_sizeSpinner->getCurrentValue() + SCALE_SPEED);
        }
        // Light intensity
        if (m_inputManager.isKeyDown(SDLK_DOWN)) {
            m_aSlider->setCurrentValue(m_aSlider->getCurrentValue() - ALPHA_SPEED);
        } else if (m_inputManager.isKeyDown(SDLK_UP)) {
            m_aSlider->setCurrentValue(m_aSlider->getCurrentValue() + ALPHA_SPEED);
        }
    }
}

void EditorScreen::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.2f, 1.0f);

    drawWorld();
    drawUI();
}

void EditorScreen::drawUI() {

    // Outlines
    if (m_selectMode == SelectionMode::PLACE && !isMouseInUI()) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        glm::vec2 pos = m_camera.convertScreenToWorld(glm::vec2(x, y));
        // Draw the object placement outlines
        if (m_objectMode == ObjectMode::PLATFORM) {
            m_debugRenderer.drawBox(glm::vec4(pos - m_boxDims * 0.5f, m_boxDims), Bengine::ColorRGBA8(255, 255, 255, 200), m_rotation);
            m_debugRenderer.end();
            m_debugRenderer.render(m_camera.getCameraMatrix(), 2.0f);
        } else if (m_objectMode == ObjectMode::LIGHT) {
            // Make temporary light to render
            Light tmpLight;
            tmpLight.position = pos;
            tmpLight.color = Bengine::ColorRGBA8(m_colorPickerRed, m_colorPickerGreen, m_colorPickerBlue, m_colorPickerAlpha);
            tmpLight.size = m_lightSize;

            // Draw light
            m_lightProgram.use();
            GLint pUniform = m_textureProgram.getUniformLocation("P");
            glUniformMatrix4fv(pUniform, 1, GL_FALSE, &m_camera.getCameraMatrix()[0][0]);
            // Additive blending
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            m_spriteBatch.begin();
            tmpLight.draw(m_spriteBatch);
            m_spriteBatch.end();
            m_spriteBatch.renderBatch();
            m_lightProgram.unuse();
            // Restore alpha blending
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Selection radius
            m_debugRenderer.drawCircle(pos, Bengine::ColorRGBA8(255, 255, 255, 255), LIGHT_SELECT_RADIUS);
            // Outer radius
            m_debugRenderer.drawCircle(pos, tmpLight.color, tmpLight.size);
            m_debugRenderer.end();
            m_debugRenderer.render(m_camera.getCameraMatrix(), 2.0f);
        }
    } else {
        // Draw selected light
        if (m_selectedLight != NO_LIGHT) {
            // Selection radius
            m_debugRenderer.drawCircle(m_lights[m_selectedLight].position, Bengine::ColorRGBA8(255, 255, 0, 255), LIGHT_SELECT_RADIUS);
            // Outer radius
            m_debugRenderer.drawCircle(m_lights[m_selectedLight].position, m_lights[m_selectedLight].color, m_lights[m_selectedLight].size);
            m_debugRenderer.end();
            m_debugRenderer.render(m_camera.getCameraMatrix(), 2.0f);
        }
        // Draw selected box
        if (m_selectedBox != NO_BOX) {
            const Box& b = m_boxes[m_selectedBox];
            glm::vec4 destRect;
            destRect.x = b.getBody()->GetPosition().x - b.getDimensions().x / 2.0f;
            destRect.y = b.getBody()->GetPosition().y - b.getDimensions().y / 2.0f;
            destRect.z = b.getDimensions().x;
            destRect.w = b.getDimensions().y;

            m_debugRenderer.drawBox(destRect, Bengine::ColorRGBA8(255, 255, 0, 255), b.getBody()->GetAngle());
            m_debugRenderer.end();
            m_debugRenderer.render(m_camera.getCameraMatrix(), 2.0f);
        }
    }

    m_textureProgram.use();

    // Camera matrix
    glm::mat4 projectionMatrix = m_uiCamera.getCameraMatrix();
    GLint pUniform = m_textureProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

    m_spriteBatch.begin();

    { // Draw the color picker quad
        const float QUAD_SIZE = 75.0f;

        glm::vec4 destRect;
        // Sorry for this
        destRect.x = m_aSlider->getXPosition().d_scale * m_window->getScreenWidth() + 10.0f - m_window->getScreenWidth() / 2.0f + QUAD_SIZE / 2.0f;
        destRect.y = m_window->getScreenHeight() / 2.0f - m_aSlider->getYPosition().d_scale * m_window->getScreenHeight() -
            m_aSlider->getHeight().d_scale * m_window->getScreenHeight() * 0.5 - QUAD_SIZE / 4.0f;
        destRect.z = QUAD_SIZE;
        destRect.w = QUAD_SIZE;
        m_spriteBatch.draw(destRect, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), m_blankTexture.id, 0.0f, Bengine::ColorRGBA8((GLubyte)m_colorPickerRed, (GLubyte)m_colorPickerGreen, (GLubyte)m_colorPickerBlue, 255));

        // Draw color text
        std::string colorText;
        if (m_objectMode == ObjectMode::LIGHT) {
            colorText = "RGBA[" + std::to_string((int)m_colorPickerRed) + "," + std::to_string((int)m_colorPickerGreen) + "," + std::to_string((int)m_colorPickerBlue) + "," + std::to_string((int)m_colorPickerAlpha) + "]";
        } else {
            colorText = "RGB[" + std::to_string((int)m_colorPickerRed) + "," + std::to_string((int)m_colorPickerGreen) + "," + std::to_string((int)m_colorPickerBlue) + "]";
        }
        m_spriteFont.draw(m_spriteBatch, colorText.c_str(), glm::vec2(destRect.x + destRect.z * 0.5, destRect.y + destRect.w), glm::vec2(0.55), 0.0f, Bengine::ColorRGBA8(255, 255, 255, 255), Bengine::Justification::MIDDLE);
    }

    // Draw custon labels for widgets
    for (auto& l : m_widgetLabels) l.draw(m_spriteBatch, m_spriteFont, m_window);

    m_spriteBatch.end();
    m_spriteBatch.renderBatch();
    m_textureProgram.unuse();

    m_gui.draw();
}

void EditorScreen::drawWorld() {
    m_textureProgram.use();

    // Upload texture uniform
    GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
    glUniform1i(textureUniform, 0);
    glActiveTexture(GL_TEXTURE0);

    // Camera matrix
    glm::mat4 projectionMatrix = m_camera.getCameraMatrix();
    GLint pUniform = m_textureProgram.getUniformLocation("P");
    glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);


    { // Draw all the boxes and the player
        m_spriteBatch.begin();
        for (auto& b : m_boxes) {
            b.draw(m_spriteBatch);
        }
        if (m_hasPlayer) m_player.draw(m_spriteBatch);

        m_spriteBatch.end();
        m_spriteBatch.renderBatch();
        m_textureProgram.unuse();
    }

    { // Draw lights
        m_lightProgram.use();
        pUniform = m_textureProgram.getUniformLocation("P");
        glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

        // Additive blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        m_spriteBatch.begin();

        for (auto& l : m_lights) l.draw(m_spriteBatch);

        m_spriteBatch.end();
        m_spriteBatch.renderBatch();

        m_lightProgram.unuse();

        // Restore alpha blending
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    // Debug rendering
    if (m_debugRender) {
        // Player collision
        if (m_hasPlayer) m_player.drawDebug(m_debugRenderer);
        // Render dynamic box borders
        glm::vec4 destRect;
        for (auto& b : m_boxes) {
            destRect.x = b.getBody()->GetPosition().x - b.getDimensions().x / 2.0f;
            destRect.y = b.getBody()->GetPosition().y - b.getDimensions().y / 2.0f;
            destRect.z = b.getDimensions().x;
            destRect.w = b.getDimensions().y;
            // Dynamic is green, static is red.
            if (b.isDynamic()) {
                m_debugRenderer.drawBox(destRect, Bengine::ColorRGBA8(0, 255, 0, 255), b.getBody()->GetAngle());
            } else {
                m_debugRenderer.drawBox(destRect, Bengine::ColorRGBA8(255, 0, 0, 255), b.getBody()->GetAngle());
            }
        }
        // Render magenta light selection radius
        for (auto& l : m_lights) {
            m_debugRenderer.drawCircle(l.position, Bengine::ColorRGBA8(255, 0, 255, 255), LIGHT_SELECT_RADIUS);
        }
        // Draw axis
        // +X axis
        m_debugRenderer.drawLine(glm::vec2(0.0f), glm::vec2(100000.0f, 0.0f), Bengine::ColorRGBA8(255, 0, 0, 200));
        // -X axis
        m_debugRenderer.drawLine(glm::vec2(0.0f), glm::vec2(-100000.0f, 0.0f), Bengine::ColorRGBA8(255, 0, 0, 100));
        // +Y axis
        m_debugRenderer.drawLine(glm::vec2(0.0f), glm::vec2(0.0f, 100000.0f), Bengine::ColorRGBA8(0, 255, 0, 200));
        // -Y axis
        m_debugRenderer.drawLine(glm::vec2(0.0f), glm::vec2(0.0f, -100000.0f), Bengine::ColorRGBA8(0, 255, 0, 100));

        m_debugRenderer.end();
        m_debugRenderer.render(m_camera.getCameraMatrix(), 2.0f);
    }
}

void EditorScreen::initUI() {
    // Init the UI
    m_gui.init("GUI");
    m_gui.loadScheme("TaharezLook.scheme");
    m_gui.setFont("DejaVuSans-10");

    // Add group box back panel
    m_groupBox = static_cast<CEGUI::GroupBox*>(m_gui.createWidget("TaharezLook/GroupBox", glm::vec4(0.001f, 0.0f, 0.18f, 0.67f), glm::vec4(0.0f), "GroupBox"));
    // Group box should be behind everything.
    m_groupBox->setAlwaysOnTop(false);
    m_groupBox->moveToBack();
    m_groupBox->disable(); // If you don't disable it, clicking on it will move it to the foreground and it will steal events from other widgets.

    { // Add the color picker
        const float X_POS = 0.01f;
        const float X_DIM = 0.015f, Y_DIM = 0.1f;
        const float Y_POS = 0.05f;
        const float PADDING = 0.005f;

        m_rSlider = static_cast<CEGUI::Slider*>(m_gui.createWidget("TaharezLook/Slider", glm::vec4(X_POS, Y_POS, X_DIM, Y_DIM), glm::vec4(0.0f), "RedSlider"));
        m_rSlider->setMaxValue(255.0f);
        m_rSlider->setCurrentValue(m_colorPickerRed);
        m_rSlider->subscribeEvent(CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber(&EditorScreen::onColorPickerRedChanged, this));
        m_rSlider->setClickStep(1.0f);

        m_gSlider = static_cast<CEGUI::Slider*>(m_gui.createWidget("TaharezLook/Slider", glm::vec4(X_POS + X_DIM + PADDING, Y_POS, X_DIM, Y_DIM), glm::vec4(0.0f), "GreenSlider"));
        m_gSlider->setMaxValue(255.0f);
        m_gSlider->setCurrentValue(m_colorPickerGreen);
        m_gSlider->subscribeEvent(CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber(&EditorScreen::onColorPickerGreenChanged, this));
        m_gSlider->setClickStep(1.0f);

        m_bSlider = static_cast<CEGUI::Slider*>(m_gui.createWidget("TaharezLook/Slider", glm::vec4(X_POS + (X_DIM + PADDING) * 2, Y_POS, X_DIM, Y_DIM), glm::vec4(0.0f), "BlueSlider"));
        m_bSlider->setMaxValue(255.0f);
        m_bSlider->setCurrentValue(m_colorPickerBlue);
        m_bSlider->subscribeEvent(CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber(&EditorScreen::onColorPickerBlueChanged, this));
        m_bSlider->setClickStep(1.0f);

        m_aSlider = static_cast<CEGUI::Slider*>(m_gui.createWidget("TaharezLook/Slider", glm::vec4(X_POS + (X_DIM + PADDING) * 3, Y_POS, X_DIM, Y_DIM), glm::vec4(0.0f), "AlphaSlider"));
        m_aSlider->setMaxValue(255.0f);
        m_aSlider->setCurrentValue(m_colorPickerAlpha);
        m_aSlider->subscribeEvent(CEGUI::Slider::EventValueChanged, CEGUI::Event::Subscriber(&EditorScreen::onColorPickerAlphaChanged, this));
        m_aSlider->setClickStep(1.0f);
    }

    { // Add Object type radio buttons
        const float X_POS = 0.02f;
        const float Y_POS = 0.20f;
        const float DIMS_PIXELS = 20.0f;
        const float PADDING = 0.042f;
        const float TEXT_SCALE = 0.6f;
        const int GROUP_ID = 1;
        m_playerRadioButton = static_cast<CEGUI::RadioButton*>(m_gui.createWidget("TaharezLook/RadioButton", glm::vec4(X_POS, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "PlayerButton"));
        m_playerRadioButton->setSelected(true);
        m_playerRadioButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onPlayerMouseClick, this));
        m_playerRadioButton->setGroupID(GROUP_ID);
        m_widgetLabels.emplace_back(m_playerRadioButton, "Player", TEXT_SCALE);

        m_platformRadioButton = static_cast<CEGUI::RadioButton*>(m_gui.createWidget("TaharezLook/RadioButton", glm::vec4(X_POS + PADDING, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "PlatformButton"));
        m_platformRadioButton->setSelected(false);
        m_platformRadioButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onPlatformMouseClick, this));
        m_platformRadioButton->setGroupID(GROUP_ID);
        m_widgetLabels.emplace_back(m_platformRadioButton, "Platform", TEXT_SCALE);

        m_finishRadioButton = static_cast<CEGUI::RadioButton*>(m_gui.createWidget("TaharezLook/RadioButton", glm::vec4(X_POS + PADDING * 2.0, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "FinishedButton"));
        m_finishRadioButton->setSelected(false);
        m_finishRadioButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onFinishMouseClick, this));
        m_finishRadioButton->setGroupID(GROUP_ID);
        m_widgetLabels.emplace_back(m_finishRadioButton, "Finish", TEXT_SCALE);

        m_lightRadioButton = static_cast<CEGUI::RadioButton*>(m_gui.createWidget("TaharezLook/RadioButton", glm::vec4(X_POS + PADDING * 3.0, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "LightButton"));
        m_lightRadioButton->setSelected(false);
        m_lightRadioButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onLightMouseClick, this));
        m_lightRadioButton->setGroupID(GROUP_ID);
        m_widgetLabels.emplace_back(m_lightRadioButton, "Light", TEXT_SCALE);

        m_objectMode = ObjectMode::PLAYER;
    }

    { // Add the physics mode radio buttons as well as rotation and size spinner
        const float X_POS = 0.02f;
        const float Y_POS = 0.28f;
        const float DIMS_PIXELS = 20.0f;
        const float PADDING = 0.04f;
        const float TEXT_SCALE = 0.7f;
        const int GROUP_ID = 2;
        m_rigidRadioButton = static_cast<CEGUI::RadioButton*>(m_gui.createWidget("TaharezLook/RadioButton", glm::vec4(X_POS, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "RigidButton"));
        m_rigidRadioButton->setSelected(true);
        m_rigidRadioButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onRigidMouseClick, this));
        m_rigidRadioButton->setGroupID(GROUP_ID);
        m_widgetLabels.emplace_back(m_rigidRadioButton, "Rigid", TEXT_SCALE);

        m_dynamicRadioButton = static_cast<CEGUI::RadioButton*>(m_gui.createWidget("TaharezLook/RadioButton", glm::vec4(X_POS + PADDING, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "DynamicButton"));
        m_dynamicRadioButton->setSelected(false);
        m_dynamicRadioButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onDynamicMouseClick, this));
        m_dynamicRadioButton->setGroupID(GROUP_ID);
        m_widgetLabels.emplace_back(m_dynamicRadioButton, "Dynamic", TEXT_SCALE);

        // Rotation spinner
        m_rotationSpinner = static_cast<CEGUI::Spinner*>(m_gui.createWidget("TaharezLook/Spinner", glm::vec4(X_POS + PADDING * 2.0, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS * 5, DIMS_PIXELS * 2), "RotationSpinner"));
        m_rotationSpinner->setMinimumValue(0.0);
        m_rotationSpinner->setMaximumValue(M_PI * 2.0);
        m_rotationSpinner->setCurrentValue(m_rotation);
        m_rotationSpinner->setStepSize(0.01);
        m_rotationSpinner->setTextInputMode(CEGUI::Spinner::FloatingPoint);
        m_rotationSpinner->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber(&EditorScreen::onRotationValueChange, this));
        m_widgetLabels.emplace_back(m_rotationSpinner, "Rotation", TEXT_SCALE);

        // Light size
        m_sizeSpinner = static_cast<CEGUI::Spinner*>(m_gui.createWidget("TaharezLook/Spinner", glm::vec4(X_POS + PADDING * 2.0, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS * 5, DIMS_PIXELS * 2), "SizeSpinner"));
        m_sizeSpinner->setMinimumValue(0.0);
        m_sizeSpinner->setMaximumValue(100.0);
        m_sizeSpinner->setCurrentValue(m_lightSize);
        m_sizeSpinner->setStepSize(0.1);
        m_sizeSpinner->setTextInputMode(CEGUI::Spinner::FloatingPoint);
        m_sizeSpinner->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber(&EditorScreen::onSizeValueChange, this));
        m_widgetLabels.emplace_back(m_sizeSpinner, "Size", TEXT_SCALE);

        m_physicsMode = PhysicsMode::RIGID;
    }

    { // Add platform dimension spinners
        const float X_POS = 0.02f;
        const float Y_POS = 0.35f;
        const float DIMS_PIXELS = 20.0f;
        const float PADDING = 0.04f;
        const float TEXT_SCALE = 0.7f;
        m_widthSpinner = static_cast<CEGUI::Spinner*>(m_gui.createWidget("TaharezLook/Spinner", glm::vec4(X_POS, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS * 5, DIMS_PIXELS * 2), "WidthSpinner"));
        m_widthSpinner->setMinimumValue(0.0);
        m_widthSpinner->setMaximumValue(10000.0);
        m_widthSpinner->setCurrentValue(m_boxDims.x);
        m_widthSpinner->setStepSize(0.1);
        m_widthSpinner->setTextInputMode(CEGUI::Spinner::FloatingPoint);
        m_widthSpinner->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber(&EditorScreen::onWidthValueChange, this));
        m_widgetLabels.emplace_back(m_widthSpinner, "Width", TEXT_SCALE);

        m_heightSpinner = static_cast<CEGUI::Spinner*>(m_gui.createWidget("TaharezLook/Spinner", glm::vec4(X_POS + PADDING * 2.0, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS * 5, DIMS_PIXELS * 2), "HeightSpinner"));
        m_heightSpinner->setMinimumValue(0.0);
        m_heightSpinner->setMaximumValue(10000.0);
        m_heightSpinner->setCurrentValue(m_boxDims.y);
        m_heightSpinner->setStepSize(0.1);
        m_heightSpinner->setTextInputMode(CEGUI::Spinner::FloatingPoint);
        m_heightSpinner->subscribeEvent(CEGUI::Spinner::EventValueChanged, CEGUI::Event::Subscriber(&EditorScreen::onHeightValueChange, this));
        m_widgetLabels.emplace_back(m_heightSpinner, "Height", TEXT_SCALE);
    }

     { // Add Selection mode radio buttons and debug render toggle
         const float X_POS = 0.03f;
         const float Y_POS = 0.45f;
         const float DIMS_PIXELS = 20.0f;
         const float PADDING = 0.04f;
         const float TEXT_SCALE = 0.7f;
         const int GROUP_ID = 3;
         m_selectRadioButton = static_cast<CEGUI::RadioButton*>(m_gui.createWidget("TaharezLook/RadioButton", glm::vec4(X_POS, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "SelectButton"));
         m_selectRadioButton->setSelected(true);
         m_selectRadioButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onSelectMouseClick, this));
         m_selectRadioButton->setGroupID(GROUP_ID);
         m_widgetLabels.emplace_back(m_selectRadioButton, "Select", TEXT_SCALE);

         m_placeRadioButton = static_cast<CEGUI::RadioButton*>(m_gui.createWidget("TaharezLook/RadioButton", glm::vec4(X_POS + PADDING, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "PlaceButton"));
         m_placeRadioButton->setSelected(false);
         m_placeRadioButton->subscribeEvent(CEGUI::RadioButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onPlaceMouseClick, this));
         m_placeRadioButton->setGroupID(GROUP_ID);
         m_widgetLabels.emplace_back(m_placeRadioButton, "Place", TEXT_SCALE);

         m_debugToggle = static_cast<CEGUI::ToggleButton*>(m_gui.createWidget("TaharezLook/Checkbox", glm::vec4(X_POS + PADDING * 2.5, Y_POS, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, DIMS_PIXELS, DIMS_PIXELS), "DebugToggle"));
         m_debugToggle->setSelected(false);
         m_debugToggle->subscribeEvent(CEGUI::ToggleButton::EventMouseClick, CEGUI::Event::Subscriber(&EditorScreen::onDebugToggleClick, this));
         m_widgetLabels.emplace_back(m_debugToggle, "Debug", TEXT_SCALE);
         m_debugRender = false;

         m_selectMode = SelectionMode::SELECT;
     }

    { // Add save and back buttons
        CEGUI::PushButton* saveButton = static_cast<CEGUI::PushButton*>(m_gui.createWidget("TaharezLook/Button", glm::vec4(0.03f, 0.5f, 0.1f, 0.05f), glm::vec4(0.0f), "SaveButton"));
        saveButton->setText("Save");
        saveButton->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&EditorScreen::onSaveMouseClick, this));

        CEGUI::PushButton* backButton = static_cast<CEGUI::PushButton*>(m_gui.createWidget("TaharezLook/Button", glm::vec4(0.03f, 0.57f, 0.1f, 0.05f), glm::vec4(0.0f), "BackButton"));
        backButton->setText("Back");
        backButton->subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&EditorScreen::onBackMouseClick, this));
    }

    setLightWidgetVisibility(false);
    setPlatformWidgetVisibility(false);
    m_gui.setMouseCursor("TaharezLook/MouseArrow");
    m_gui.showMouseCursor();
    SDL_ShowCursor(0);
}

void EditorScreen::checkInput() {
    SDL_Event evnt;
    while (SDL_PollEvent(&evnt)) {
        m_gui.onSDLEvent(evnt);
        switch (evnt.type) {
            case SDL_QUIT:
                onExitClicked(CEGUI::EventArgs());
                break;
            case SDL_MOUSEBUTTONDOWN:
                updateMouseDown(evnt);
                break;
            case SDL_MOUSEBUTTONUP:
                updateMouseUp(evnt);
                break;
            case SDL_MOUSEMOTION:
                updateMouseMotion(evnt);
                break;
            case SDL_MOUSEWHEEL:
                // Linear scaling sucks for mouse wheel zoom so we multiply by getScale() instead.
                m_camera.offsetScale(m_camera.getScale() * evnt.wheel.y * 0.1f);
                break;
            case SDL_KEYDOWN:
                m_inputManager.pressKey(evnt.key.keysym.sym);
                break;
            case SDL_KEYUP:
                m_inputManager.releaseKey(evnt.key.keysym.sym);
                break;
        }
    }

    m_inputManager.update();
}

bool inLightSelect(const Light& l, const glm::vec2& pos) {
    return (glm::length(pos - l.position) <= LIGHT_SELECT_RADIUS);
}

void EditorScreen::updateMouseDown(const SDL_Event& evnt) {
    // Texture for boxes. Its here because lazy.
    static Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Assets/bricks_top.png");
    glm::vec2 pos;
    glm::vec4 uvRect;

    switch (evnt.button.button) {
    case SDL_BUTTON_LEFT:
        m_mouseButtons[MOUSE_LEFT] = true;
        // Don't place objects or select things when clicking on UI.
        if (isMouseInUI()) break;
        if (m_selectMode == SelectionMode::SELECT) {
            // Select mode
            pos = m_camera.convertScreenToWorld(glm::vec2(evnt.button.x, evnt.button.y));
            // Lights have selection priority, so check lights first
            // If a light is already selected, test to see if we just clicked it again
            if (m_selectedLight != NO_LIGHT && inLightSelect(m_lights[m_selectedLight], pos)) {
                // We selected ourself, do nothing.
            } else {
                // Unselect9
                // Find the box that we are selecting
                for (size_t i = 0; i < m_lights.size(); i++) {
                    if (inLightSelect(m_lights[i], pos)) {
                        m_selectedLight = i;
                        break;
                    }
                }
            }
            // If we selected a light
            if (m_selectedLight != NO_LIGHT) {
                // Get the offset from the center so we can drag correctly
                m_selectOffset = pos - m_lights[m_selectedLight].position;
                m_selectedBox = NO_LIGHT;
                m_isDragging = true;
                // Set widget values
                m_rSlider->setCurrentValue(m_lights[m_selectedLight].color.r);
                m_gSlider->setCurrentValue(m_lights[m_selectedLight].color.g);
                m_bSlider->setCurrentValue(m_lights[m_selectedLight].color.b);
                m_aSlider->setCurrentValue(m_lights[m_selectedLight].color.a);
                m_sizeSpinner->setCurrentValue(m_lights[m_selectedLight].size);
                m_lightRadioButton->setSelected(true);
                m_objectMode = ObjectMode::LIGHT;
                break;
            }
            // If a box is already selected, test to see if we just clicked it again
            if (m_selectedBox != NO_BOX && m_boxes[m_selectedBox].testPoint(pos.x, pos.y)) {
                // We selected ourself, do nothing.
            } else {
                // Unselect
                m_selectedBox = NO_BOX;
                // Find the box that we are selecting
                for (size_t i = 0; i < m_boxes.size(); i++) {
                    if (m_boxes[i].testPoint(pos.x, pos.y)) {
                        m_selectedBox = i;
                        break;
                    }
                }
            }
            // If we selected a box
            if (m_selectedBox != NO_BOX) {
                // Get the offset from the center so we can drag correctly
                m_selectOffset = pos - m_boxes[m_selectedBox].getPosition();
                m_isDragging = true;
                // Set widget values
                m_rotationSpinner->setCurrentValue(m_boxes[m_selectedBox].getBody()->GetAngle());
                m_widthSpinner->setCurrentValue(m_boxes[m_selectedBox].getDimensions().x);
                m_heightSpinner->setCurrentValue(m_boxes[m_selectedBox].getDimensions().y);
                m_rSlider->setCurrentValue(m_boxes[m_selectedBox].getColor().r);
                m_gSlider->setCurrentValue(m_boxes[m_selectedBox].getColor().g);
                m_bSlider->setCurrentValue(m_boxes[m_selectedBox].getColor().b);
                if (m_boxes[m_selectedBox].isDynamic()) {
                    m_dynamicRadioButton->setSelected(true);
                    m_physicsMode = PhysicsMode::DYNAMIC;
                } else {
                    m_rigidRadioButton->setSelected(true);
                    m_physicsMode = PhysicsMode::RIGID;
                }
                m_platformRadioButton->setSelected(true);
                m_objectMode = ObjectMode::PLATFORM;
            }
        } else {
            // Place mode
            Box newBox;
            Light newLight;
            // Place
            switch (m_objectMode) {
            case ObjectMode::PLATFORM:
                pos = m_camera.convertScreenToWorld(glm::vec2(evnt.button.x, evnt.button.y));
                uvRect.x = pos.x;
                uvRect.y = pos.y;
                uvRect.z = m_boxDims.x;
                uvRect.w = m_boxDims.y;
                newBox.init(m_world.get(), pos, m_boxDims, texture, Bengine::ColorRGBA8(m_colorPickerRed, m_colorPickerGreen, m_colorPickerBlue, 255),
                            false, m_physicsMode == PhysicsMode::DYNAMIC, m_rotation, uvRect);
                m_boxes.push_back(newBox);
                break;
            case ObjectMode::PLAYER:
                // Just remove the player, easiest way.
                m_player.destroy(m_world.get());
                // Re-init the player
                pos = m_camera.convertScreenToWorld(glm::vec2(evnt.button.x, evnt.button.y));
                m_player.init(m_world.get(), pos, glm::vec2(2.0f), glm::vec2(1.0f, 1.8f), Bengine::ColorRGBA8(m_colorPickerRed, m_colorPickerGreen, m_colorPickerBlue, 255));
                m_hasPlayer = true;
                break;
            case ObjectMode::LIGHT:
                newLight.position = m_camera.convertScreenToWorld(glm::vec2(evnt.button.x, evnt.button.y));
                newLight.size = m_lightSize;
                newLight.color = Bengine::ColorRGBA8(m_colorPickerRed, m_colorPickerGreen, m_colorPickerBlue, m_colorPickerAlpha);
                m_lights.push_back(newLight);
                break;
            case ObjectMode::FINISH:
                // TODO: Implement
                break;
            }
        }
        break;
    case SDL_BUTTON_RIGHT:
        m_mouseButtons[MOUSE_RIGHT] = true;
        break;
    }

}

void EditorScreen::updateMouseUp(const SDL_Event& evnt) {
    switch (evnt.button.button) {
        case SDL_BUTTON_LEFT:
            m_mouseButtons[MOUSE_LEFT] = false;
            break;
        case SDL_BUTTON_RIGHT:
            m_mouseButtons[MOUSE_RIGHT] = false;
            break;
    }
    m_isDragging = false;
}

void EditorScreen::updateMouseMotion(const SDL_Event& evnt) {
    // If right button is down, drag camera
    const float SPEED = 0.05f;
    if (m_mouseButtons[MOUSE_RIGHT]) {
        m_camera.offsetPosition(glm::vec2(-evnt.motion.xrel, evnt.motion.yrel * m_camera.getAspectRatio()) * SPEED);
    }
    // Dragging stuff around
    if (m_isDragging && m_mouseButtons[MOUSE_LEFT]) {
        // Light
        if (m_selectedLight != NO_LIGHT) {
            glm::vec2 pos = m_camera.convertScreenToWorld(glm::vec2(evnt.motion.x, evnt.motion.y)) - m_selectOffset;
            refreshSelectedLight(pos);
        }
        // Box
        if (m_selectedBox != NO_BOX) {
            glm::vec2 pos = m_camera.convertScreenToWorld(glm::vec2(evnt.motion.x, evnt.motion.y)) - m_selectOffset;
            refreshSelectedBox(pos);
        }
    }
}

void EditorScreen::refreshSelectedBox() {
    if (m_selectedBox == NO_BOX) return;
    refreshSelectedBox(m_boxes[m_selectedBox].getPosition());
}

void EditorScreen::refreshSelectedBox(const glm::vec2& newPosition) {
    if (m_selectedBox == NO_BOX) return;
    // Texture for boxes. Its here because lazy.
    static Bengine::GLTexture texture = Bengine::ResourceManager::getTexture("Assets/bricks_top.png");
    glm::vec4 uvRect;
    uvRect.x = newPosition.x;
    uvRect.y = newPosition.y;
    uvRect.z = m_boxDims.x;
    uvRect.w = m_boxDims.y;
    Box newBox;

    newBox.init(m_world.get(), newPosition, m_boxDims, texture, Bengine::ColorRGBA8(m_colorPickerRed, m_colorPickerGreen, m_colorPickerBlue, 255),
                false, m_physicsMode == PhysicsMode::DYNAMIC, m_rotation, uvRect);
    // Destroy old box and replace with new one
    m_boxes[m_selectedBox].destroy(m_world.get());
    m_boxes[m_selectedBox] = newBox;
}

void EditorScreen::refreshSelectedLight() {
    if (m_selectedLight == NO_LIGHT) return;
    refreshSelectedBox(m_lights[m_selectedLight].position);
}

void EditorScreen::refreshSelectedLight(const glm::vec2& newPosition) {
    if (m_selectedLight == NO_LIGHT) return;
    Light newLight;
    newLight.position = newPosition;
    newLight.size = m_lightSize;
    newLight.color = Bengine::ColorRGBA8(m_colorPickerRed, m_colorPickerGreen, m_colorPickerBlue, m_colorPickerAlpha);
    m_lights[m_selectedLight] = newLight;
}

// This is definitely not the best way to do this, but it works!
bool EditorScreen::isMouseInUI() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    const float SW = m_window->getScreenWidth();
    const float SH = m_window->getScreenHeight();
    // Notice we aren't converting to world space, we are staying in screen space because UI.
    return (x >= m_groupBox->getXPosition().d_scale * SW && x <= m_groupBox->getXPosition().d_scale * SW + m_groupBox->getWidth().d_scale  * SW &&
            y >= m_groupBox->getYPosition().d_scale * SH && y <= m_groupBox->getYPosition().d_scale * SH + m_groupBox->getHeight().d_scale * SH);
}

void EditorScreen::setPlatformWidgetVisibility(bool visible) {
    m_rigidRadioButton->setVisible(visible);
    m_dynamicRadioButton->setVisible(visible);
    m_rotationSpinner->setVisible(visible);
    m_widthSpinner->setVisible(visible);
    m_heightSpinner->setVisible(visible);
}

void EditorScreen::setLightWidgetVisibility(bool visible) {
    m_aSlider->setVisible(visible);
    m_sizeSpinner->setVisible(visible);
}

bool EditorScreen::onExitClicked(const CEGUI::EventArgs& e) {
    m_currentState = Bengine::ScreenState::EXIT_APPLICATION;
    return true;
}

bool EditorScreen::onColorPickerRedChanged(const CEGUI::EventArgs& e) {
    m_colorPickerRed = m_rSlider->getCurrentValue();
    refreshSelectedBox();
    refreshSelectedLight();
    return true;
}

bool EditorScreen::onColorPickerGreenChanged(const CEGUI::EventArgs& e) {
    m_colorPickerGreen = m_gSlider->getCurrentValue();
    refreshSelectedBox();
    refreshSelectedLight();
    return true;
}

bool EditorScreen::onColorPickerBlueChanged(const CEGUI::EventArgs& e) {
    m_colorPickerBlue = m_bSlider->getCurrentValue();
    refreshSelectedBox();
    refreshSelectedLight();
    return true;
}

bool EditorScreen::onColorPickerAlphaChanged(const CEGUI::EventArgs& e) {
    m_colorPickerAlpha = m_aSlider->getCurrentValue();
    refreshSelectedBox();
    refreshSelectedLight();
    return true;
}

bool EditorScreen::onRigidMouseClick(const CEGUI::EventArgs& e) {
    m_physicsMode = PhysicsMode::RIGID;
    refreshSelectedBox();
    return true;
}

bool EditorScreen::onDynamicMouseClick(const CEGUI::EventArgs& e) {
    m_physicsMode = PhysicsMode::DYNAMIC;
    refreshSelectedBox();
    return true;
}

bool EditorScreen::onPlayerMouseClick(const CEGUI::EventArgs& e) {
    m_objectMode = ObjectMode::PLAYER;
    setPlatformWidgetVisibility(false);
    setLightWidgetVisibility(false);
    return true;
}

bool EditorScreen::onPlatformMouseClick(const CEGUI::EventArgs& e) {
    m_objectMode = ObjectMode::PLATFORM;
    setPlatformWidgetVisibility(true);
    setLightWidgetVisibility(false);
    return true;
}

bool EditorScreen::onFinishMouseClick(const CEGUI::EventArgs& e) {
    m_objectMode = ObjectMode::FINISH;
    setPlatformWidgetVisibility(false);
    setLightWidgetVisibility(false);
    return true;
}

bool EditorScreen::onLightMouseClick(const CEGUI::EventArgs& e) {
    m_objectMode = ObjectMode::LIGHT;
    setPlatformWidgetVisibility(false);
    setLightWidgetVisibility(true);
    return true;
}

bool EditorScreen::onSelectMouseClick(const CEGUI::EventArgs& e) {
    m_selectMode = SelectionMode::SELECT;
    return true;
}

bool EditorScreen::onPlaceMouseClick(const CEGUI::EventArgs& e) {
    m_selectMode = SelectionMode::PLACE;
    m_selectedBox = NO_BOX;
    m_selectedLight = NO_LIGHT;
    return true;
}

bool EditorScreen::onSaveMouseClick(const CEGUI::EventArgs& e) {
    puts("EditorScreen::onSaveMouseClick() not implemented");
    return true;
}

bool EditorScreen::onBackMouseClick(const CEGUI::EventArgs& e) {
    m_currentState = Bengine::ScreenState::CHANGE_PREVIOUS;
    return true;
}

bool EditorScreen::onRotationValueChange(const CEGUI::EventArgs& e) {
    m_rotation = m_rotationSpinner->getCurrentValue();
    refreshSelectedBox();
    return true;
}

bool EditorScreen::onSizeValueChange(const CEGUI::EventArgs& e) {
    m_lightSize = m_sizeSpinner->getCurrentValue();
    refreshSelectedLight();
    return true;
}

bool EditorScreen::onWidthValueChange(const CEGUI::EventArgs& e) {
    m_boxDims.x = m_widthSpinner->getCurrentValue();
    refreshSelectedBox();
    return true;
}

bool EditorScreen::onHeightValueChange(const CEGUI::EventArgs& e) {
    m_boxDims.y = m_heightSpinner->getCurrentValue();
    refreshSelectedBox();
    return true;
}

bool EditorScreen::onDebugToggleClick(const CEGUI::EventArgs& e) {
    m_debugRender = m_debugToggle->isSelected();
    return true;
}