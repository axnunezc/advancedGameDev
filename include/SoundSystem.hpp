#ifndef SOUNDSYSTEM_HPP
#define SOUNDSYSTEM_HPP

#include <SDL.h>
#include <vector>
#include <string>
#include <memory>
#include <iostream>

// Represents a preloaded sound with RAII management
class Sound {
private:
    Uint8* buffer;
    Uint32 length;
    
public:
    Sound(Uint8* b, Uint32 l) : buffer(b), length(l) {}
    ~Sound() {
        if (buffer) {
            SDL_FreeWAV(buffer);
        }
    }
    
    // Prevent copying to maintain RAII semantics
    Sound(const Sound&) = delete;
    Sound& operator=(const Sound&) = delete;
    
    // Allow moving
    Sound(Sound&& other) noexcept : buffer(other.buffer), length(other.length) {
        other.buffer = nullptr;
        other.length = 0;
    }
    
    Sound& operator=(Sound&& other) noexcept {
        if (this != &other) {
            if (buffer) {
                SDL_FreeWAV(buffer);
            }
            buffer = other.buffer;
            length = other.length;
            other.buffer = nullptr;
            other.length = 0;
        }
        return *this;
    }
    
    Uint8* getBuffer() const { return buffer; }
    Uint32 getLength() const { return length; }
};

// Represents an actively playing sound
struct SoundState {
    Uint8* buffer;    // Raw audio buffer
    Uint32 length;    // Total buffer length
    Uint32 position;  // Current playback position
    bool playing;     // Playback flag
    bool autoFree;    // Whether to free the buffer when finished
    
    SoundState() : buffer(nullptr), length(0), position(0), playing(false), autoFree(false) {}
};

// Core Sound System
class SoundSystem {
private:
    SDL_AudioDeviceID deviceID;
    SDL_AudioSpec audioSpec;
    std::vector<std::unique_ptr<Sound>> sounds;  // Preloaded sounds using RAII
    std::vector<SoundState> activeSounds;        // Playing sounds
    Uint8* mixBuffer;                            // Mixing buffer
    int mixBufferSize;                           // Buffer size
    
    static void audioCallback(void* userdata, Uint8* stream, int len);

public:
    SoundSystem();
    ~SoundSystem();
    
    // Disable copying
    SoundSystem(const SoundSystem&) = delete;
    SoundSystem& operator=(const SoundSystem&) = delete;
    
    // Load a sound into the sounds library for repeated use
    bool loadSound(const std::string& filepath);
    
    // Play a sound from the library (by index)
    void playSound(int soundIndex);
    
    // Play a sound directly from file (one-time use)
    void playSound(const std::string& filepath);

    void cleanup();
};

#endif // SOUNDSYSTEM_HPP