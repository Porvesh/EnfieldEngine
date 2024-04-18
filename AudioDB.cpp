// AudioDB.h

#include "headers/AudioDB.h"

std::unordered_map<std::string, Mix_Chunk*> AudioDB::audioClips;

AudioDB::AudioDB() {
    // Initialize SDL_mixer with some standard audio format, frequency, and channels
    if (AudioHelper::Mix_OpenAudio498(44100, MIX_DEFAULT_FORMAT, 1, 1024) == -1) {
        std::cout << "Mix_OpenAudio: " << Mix_GetError() ;
    }
    AudioHelper::Mix_AllocateChannels498(50);
}

AudioDB::~AudioDB() {
    Mix_CloseAudio(); // Close the mixer
}


void AudioDB::LoadSoundEffect(const std::string& storeVariable) {

    if (audioClips.find(storeVariable) != audioClips.end()) {
        return;
    }

    std::string wavPath = "resources/audio/" + storeVariable + ".wav";
    std::string oggPath = "resources/audio/" + storeVariable + ".ogg";

    // Attempt to load the .wav file first
    Mix_Chunk* sound = AudioHelper::Mix_LoadWAV498(wavPath.c_str());
    if (!sound) {
        // If .wav loading fails, attempt to load the .ogg file
        sound = AudioHelper::Mix_LoadWAV498(oggPath.c_str());
    }
    if (sound) {
        audioClips[storeVariable] = sound;
    }
}

void AudioDB::PlaySoundEffect(int channel, const std::string& effectName, bool doesLoop){
    auto it = audioClips.find(effectName);
    if (it != audioClips.end()) {
        AudioHelper::Mix_PlayChannel498(channel, it->second, doesLoop ? -1 : 0);
    }
    else {
        LoadSoundEffect(effectName);
        auto it2 = audioClips.find(effectName);
        AudioHelper::Mix_PlayChannel498(channel, it2->second, doesLoop ? -1 : 0);
    }
}

void AudioDB::Halt(int channel) {
    AudioHelper::Mix_HaltChannel498(channel);
}

void AudioDB::SetVolume(int channel, float volume) {
    // Ensure volume is within SDL_mixer's range
    volume = std::max(0, std::min(static_cast<int>(volume), 128));
    AudioHelper::Mix_Volume498(channel, volume);
}

void AudioDB::clearAll() {
    audioClips.clear();
}