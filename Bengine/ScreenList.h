#pragma once

#include <vector>

namespace Bengine {

    class IMainGame;
    class IGameScreen;

    class ScreenList {
    public:
        ScreenList(IMainGame* game);
        ~ScreenList();

        IGameScreen* moveNext();
        IGameScreen* movePrevious();

        void setScreen(int nextScreen);
        void addScreen(IGameScreen* newScreen);

        void destroy();

        IGameScreen* getCurrent();

    protected:
        IMainGame* m_game = nullptr;
        std::vector<IGameScreen*> m_screens;
        int m_currentScreenIndex = -1;
    };

}