#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <cstdint>

namespace Engine {
    void initialize();  // Engine initialization function
    void update();      // NEW: Updates delta time each frame
    std::uint32_t getDeltaTime();   // Returns delta time in milliseconds
    float getDeltaSeconds();        // Returns delta time in seconds
}

#endif
