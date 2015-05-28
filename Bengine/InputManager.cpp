#include "InputManager.h"

namespace Bengine {

InputManager::InputManager() : _mouseCoords(0.0f)
{
}


InputManager::~InputManager()
{
}

void InputManager::update() {
    // Loop through _keyMap using a for each loop, and copy it over to _previousKeyMap
    for (auto& it : _keyMap) {
        _previousKeyMap[it.first] = it.second;
    }
}

void InputManager::pressKey(unsigned int keyID) {
    // Here we are treating _keyMap as an associative array.
    // if keyID doesn't already exist in _keyMap, it will get added
    _keyMap[keyID] = true;
}

void InputManager::releaseKey(unsigned int keyID) {
    _keyMap[keyID] = false;
}

void InputManager::setMouseCoords(float x, float y) {
    _mouseCoords.x = x;
    _mouseCoords.y = y;
}

bool InputManager::isKeyDown(unsigned int keyID) {
    // We dont want to use the associative array approach here
    // because we don't want to create a key if it doesnt exist.
    // So we do it manually
    auto it = _keyMap.find(keyID);
    if (it != _keyMap.end()) {
        // Found the key
        return it->second;
    } else {
        // Didn't find the key
        return false;
    }
}

bool InputManager::isKeyPressed(unsigned int keyID) {
    // Check if it is pressed this frame, and wasn't pressed last frame
    if (isKeyDown(keyID) == true && wasKeyDown(keyID) == false) {
        return true;
    }
    return false;
}

bool InputManager::wasKeyDown(unsigned int keyID) {
    // We dont want to use the associative array approach here
    // because we don't want to create a key if it doesnt exist.
    // So we do it manually
    auto it = _previousKeyMap.find(keyID);
    if (it != _previousKeyMap.end()) {
        // Found the key
        return it->second;
    } else {
        // Didn't find the key
        return false;
    }
}

}