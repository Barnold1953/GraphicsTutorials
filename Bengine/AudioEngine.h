#pragma once

#include <SDL/SDL_mixer.h>
#include <string>
#include <map>

namespace Bengine {

    class SoundEffect {
    public:
        friend class AudioEngine;

        /// Plays the effect file
        /// @param loops: If loops == -1, loop forever,
        /// otherwise play it loops+1 times 
        void play(int loops = 0);
    private:
        Mix_Chunk* m_chunk = nullptr;
    };

    class Music {
    public:
        friend class AudioEngine;

        /// Plays the music file
        /// @param loops: If loops == -1, loop forever,
        /// otherwise play it loops times 
        void play(int loops = 1);

        /// Pauses whatever song is currently playing
        static void pause();
        /// Stops whatever song is currently playing
        static void stop();
        /// Resumes whatever song is currently playing
        static void resume();
    private:
        Mix_Music* m_music = nullptr;
    };

    class AudioEngine {
    public:
        AudioEngine();
        ~AudioEngine();

        void init();
        void destroy();

        SoundEffect loadSoundEffect(const std::string& filePath);
        Music loadMusic(const std::string& filePath);
    private:

        std::map<std::string, Mix_Chunk*> m_effectMap; ///< Effects cache
        std::map<std::string, Mix_Music*> m_musicMap; ///< Music cache

        bool m_isInitialized = false;
    };

}