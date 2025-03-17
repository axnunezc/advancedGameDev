#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <chrono>
#include <cstdint>

// Internal variables
namespace Utility {
    extern std::uint32_t deltaTime;   // Milliseconds
    extern float deltaSeconds;        // Seconds
    extern std::chrono::steady_clock::time_point prevTime;
    
    void updateDeltaTime(); // Updates delta time
}

#endif
