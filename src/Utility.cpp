#include "Utility.hpp"

namespace Utility {
    std::uint32_t deltaTime = 0;  // Milliseconds
    float deltaSeconds = 0.0f;    // Seconds
    std::chrono::steady_clock::time_point prevTime = std::chrono::steady_clock::now();

    void updateDeltaTime() {
        auto currentTime = std::chrono::steady_clock::now();
        deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevTime).count();
        deltaSeconds = deltaTime / 1000.0f;
        prevTime = currentTime;
    }
}
