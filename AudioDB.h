#ifndef AUDIODB_H
#define AUDIODB_H

#include <string>
#include <unordered_map>
#include "ThirdParty/SDL2_mixer/SDL_mixer.h" 

class AudioDB {
public:
    static void Init();

    // Lua API Functions
    static void Play(int channel, std::string clip_name, bool does_loop);
    static void Halt(int channel);
    static void SetVolume(int channel, float volume);

    static void Quit();

private:
    // Cache to store loaded audio clips
    static std::unordered_map<std::string, Mix_Chunk*> sound_cache;
};

#endif