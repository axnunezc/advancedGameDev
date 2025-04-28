#ifndef PHYSICS_INTEGRATOR_HPP
#define PHYSICS_INTEGRATOR_HPP

#include "GameObject.hpp"
#include <glm/glm.hpp>

namespace Physics {
    // Global gravity vector
    extern glm::vec3 gravity;
    
    // Linear integration
    glm::vec3 integrateLinear(float deltaTime, const glm::vec3& linear);
    
    // Angular integration
    Quaternion integrateAngular(float deltaTime, const glm::vec3& angular);
    
    // Apply acceleration to an object
    void integrateAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel);
    
    // Apply angular acceleration to an object
    void integrateAngularAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel);
    
    // Apply a linear impulse to an object
    void applyLinearImpulse(GameObject* obj, const glm::vec3& impulse);
    
    // Apply an angular impulse to an object
    void applyAngularImpulse(GameObject* obj, const glm::vec3& impulse);
    
    // Main update function for physics objects
    void updateObject(GameObject* obj, float deltaTime, bool applyGravity = true);
    
    // Run a simple physics test
    void runPhysicsTest();
}

#endif // PHYSICS_INTEGRATOR_HPP