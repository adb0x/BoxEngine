#include "AudioDB.h"
#include "AudioHelper.h"
#include <filesystem>
#include <iostream>

std::unordered_map<std::string, Mix_Chunk*> AudioDB::sound_cache;

void AudioDB::Init() {
    AudioHelper::Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    AudioHelper::Mix_AllocateChannels(50);
}

void AudioDB::Play(int channel, std::string clip_name, bool does_loop) {

    if (sound_cache.find(clip_name) == sound_cache.end()) {
        std::string base_path = "resources/audio/" + clip_name;
        std::string final_path = "";

        // Check for .ogg first, then .wav
        if (std::filesystem::exists(base_path + ".ogg")) {
            final_path = base_path + ".ogg";
        }
        else if (std::filesystem::exists(base_path + ".wav")) {
            final_path = base_path + ".wav";
        }

        if (final_path == "") return;

        Mix_Chunk* chunk = AudioHelper::Mix_LoadWAV(final_path.c_str());

        // If the helper fails, we cannot proceed with PlayChannel
        if (!chunk) return;

        sound_cache[clip_name] = chunk;
    }

    int loops = does_loop ? -1 : 0;

    AudioHelper::Mix_PlayChannel(channel, sound_cache[clip_name], loops);
}

void AudioDB::Halt(int channel) {
    AudioHelper::Mix_HaltChannel(channel);
}

void AudioDB::SetVolume(int channel, float volume) {
    AudioHelper::Mix_Volume(channel, (int)volume);
}

void AudioDB::Quit() {
    sound_cache.clear();
    AudioHelper::Mix_CloseAudio();
}