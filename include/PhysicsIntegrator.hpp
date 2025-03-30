#ifndef PHYSICS_INTEGRATOR_HPP
#define PHYSICS_INTEGRATOR_HPP

// Define GLM_ENABLE_EXPERIMENTAL before including any experimental GLM features
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include "Quaternion.hpp"

// Forward declaration
class GameObject;

// Namespace for physics-related functions
namespace Physics {
    // Global gravity vector (can be modified by the application)
    extern glm::vec3 gravity;

    // Basic integration functions - these don't modify GameObjects directly
    glm::vec3 integrateLinear(float deltaTime, const glm::vec3& linear);
    Quaternion integrateAngular(float deltaTime, const glm::vec3& angular);

    // These functions directly modify GameObjects
    void integrateAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel);
    void integrateAngularAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel);
    void applyLinearImpulse(GameObject* obj, const glm::vec3& impulse);
    void applyAngularImpulse(GameObject* obj, const glm::vec3& impulse);
    void updateObject(GameObject* obj, float deltaTime, bool applyGravity = true);
    
    // AABB update function
    void updateAABB(GameObject* obj);
    
    // Simple physics test function
    void runPhysicsTest();
}

#endif // PHYSICS_INTEGRATOR_HPP