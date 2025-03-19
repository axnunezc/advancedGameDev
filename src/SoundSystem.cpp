#include "SoundSystem.hpp"

void SoundSystem::audioCallback(void* userdata, Uint8* stream, int len) {
    static int callCount = 0;
    if (callCount++ % 100 == 0) {
        std::cout << "Audio callback called, len: " << len << std::endl;
    }
    
    SoundSystem* system = static_cast<SoundSystem*>(userdata);
    
    // Initialize stream to silence
    SDL_memset(stream, 0, len);
    
    // Count active sounds
    int activeCount = 0;
    for (auto& state : system->activeSounds) {
        if (state.playing) activeCount++;
    }
    
    if (activeCount > 0 && callCount % 100 == 0) {
        std::cout << "Active sounds: " << activeCount << std::endl;
    }
    
    // Process each active sound
    for (auto& state : system->activeSounds) {
        if (!state.playing) continue;
        
        // Calculate how many bytes we can mix
        Uint32 bytesLeft = state.length - state.position;
        int bytesToMix = (bytesLeft < len) ? bytesLeft : len;
        
        if (callCount % 100 == 0) {
            std::cout << "Mixing " << bytesToMix << " bytes, position: " << state.position 
                      << ", length: " << state.length << std::endl;
        }
        
        // Use direct copying instead of SDL_MixAudio to debug
        SDL_MixAudioFormat(stream, state.buffer + state.position, 
                          AUDIO_S16, bytesToMix, SDL_MIX_MAXVOLUME);
        
        // Update position
        state.position += bytesToMix;
        
        // Mark as not playing if done
        if (state.position >= state.length) {
            state.playing = false;
            if (callCount % 100 == 0) {
                std::cout << "Sound finished playing" << std::endl;
            }
        }
    }
}

SoundSystem::SoundSystem() : deviceID(0), mixBuffer(nullptr), mixBufferSize(0) {
    SDL_Init(SDL_INIT_AUDIO);
    SDL_zero(audioSpec);
    
    // Use 16-bit signed audio instead of 8-bit unsigned
    audioSpec.freq = 44100;
    audioSpec.format = AUDIO_S16LSB;
        audioSpec.channels = 2;
    audioSpec.samples = 1024;      // Smaller buffer (was 4096)
    audioSpec.callback = audioCallback;
    audioSpec.userdata = this;
    
    deviceID = SDL_OpenAudioDevice(nullptr, 0, &audioSpec, nullptr, 0);
    if (deviceID == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
    } else {
        // Allocate mix buffer (twice the size of the audio buffer for stereo, 2 bytes per sample)
        mixBufferSize = audioSpec.samples * audioSpec.channels * 2;
        mixBuffer = new Uint8[mixBufferSize];
        
        SDL_PauseAudioDevice(deviceID, 0);
    }
}

SoundSystem::~SoundSystem() {
    // Pause the audio device as specified
    SDL_PauseAudioDevice(deviceID, 1);
    
    // Close the audio device
    SDL_CloseAudioDevice(deviceID);
    
    // Delete mix buffer
    delete[] mixBuffer;
    
    // Sound objects will clean up themselves thanks to unique_ptr and RAII
    
    // For any active sounds that have autoFree set
    for (auto& state : activeSounds) {
        if (state.autoFree && state.buffer) {
            SDL_FreeWAV(state.buffer);
        }
    }
    
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

bool SoundSystem::loadSound(const std::string& filepath) {
    SDL_AudioSpec wavSpec;
    Uint8* wavBuffer;
    Uint32 wavLength;
    
    if (!SDL_LoadWAV(filepath.c_str(), &wavSpec, &wavBuffer, &wavLength)) {
        std::cerr << "Error: Failed to load WAV file: " << filepath << std::endl;
        return -1;
    }
    
    if (wavSpec.channels != 1) {
        std::cerr << "Error: Only mono WAV files are supported!" << std::endl;
        SDL_FreeWAV(wavBuffer);
        return -1;
    }
    
    // Add debug information
    std::cout << "Loaded WAV file: " << filepath << std::endl;
    std::cout << "  Format: " << wavSpec.format << std::endl;
    std::cout << "  Frequency: " << wavSpec.freq << std::endl;
    std::cout << "  Channels: " << wavSpec.channels << std::endl;
    std::cout << "  Length: " << wavLength << " bytes" << std::endl;
    
    // Create and add sound object
    sounds.push_back(std::make_unique<Sound>(wavBuffer, wavLength));
    return sounds.size() - 1;
}

void SoundSystem::playSound(int soundIndex) {
    if (soundIndex < 0 || soundIndex >= sounds.size()) {
        std::cerr << "Invalid sound index: " << soundIndex << std::endl;
        return;
    }
    
    // Lock audio before modifying the active sounds
    SDL_LockAudioDevice(deviceID);
    
    SoundState newState;
    newState.buffer = sounds[soundIndex]->getBuffer();
    newState.length = sounds[soundIndex]->getLength();
    newState.position = 0;
    newState.playing = true;
    newState.autoFree = false;  // We don't free preloaded sounds
    
    activeSounds.push_back(newState);
    
    SDL_UnlockAudioDevice(deviceID);
}

void SoundSystem::playSound(const std::string& filepath) {
    SDL_AudioSpec wavSpec;
    Uint8* wavBuffer;
    Uint32 wavLength;
    
    if (!SDL_LoadWAV(filepath.c_str(), &wavSpec, &wavBuffer, &wavLength)) {
        std::cerr << "Error: Failed to load WAV file: " << filepath << std::endl;
        return;
    }
    
    if (wavSpec.channels != 1) {
        std::cerr << "Error: Only mono WAV files are supported!" << std::endl;
        SDL_FreeWAV(wavBuffer);
        return;
    }
    
    // Lock audio before modifying the active sounds
    SDL_LockAudioDevice(deviceID);
    
    SoundState newState;
    newState.buffer = wavBuffer;
    newState.length = wavLength;
    newState.position = 0;
    newState.playing = true;
    newState.autoFree = true;  // We will free this buffer when done
    
    activeSounds.push_back(newState);
    
    SDL_UnlockAudioDevice(deviceID);
}

void SoundSystem::cleanup() {
    SDL_LockAudioDevice(deviceID);
    
    auto it = activeSounds.begin();
    while (it != activeSounds.end()) {
        if (!it->playing) {
            if (it->autoFree && it->buffer) {
                SDL_FreeWAV(it->buffer);
                it->buffer = nullptr;
            }
            it = activeSounds.erase(it);
        } else {
            ++it;
        }
    }
    
    SDL_UnlockAudioDevice(deviceID);
}