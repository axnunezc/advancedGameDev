#include "Engine.hpp"
#include "Utility.hpp"

namespace Engine {
    void initialize() {
        Utility::prevTime = std::chrono::steady_clock::now();
    }

    void update() {
        Utility::updateDeltaTime();
    }

    std::uint32_t getDeltaTime() {
        return Utility::deltaTime;
    }

    float getDeltaSeconds() {
        return Utility::deltaSeconds;
    }
}
