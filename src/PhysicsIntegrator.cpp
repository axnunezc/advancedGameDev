#include "PhysicsIntegrator.hpp"
#include "GameObject.hpp"
#include <iostream>

// Define GLM_ENABLE_EXPERIMENTAL before including experimental GLM headers
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace Physics {
    // Initialize global gravity vector
    glm::vec3 gravity(0.0f, -9.81f, 0.0f);

    glm::vec3 integrateLinear(float deltaTime, const glm::vec3& linear) {
        // Simple linear integration (position += velocity * time)
        return linear * deltaTime;
    }

    Quaternion integrateAngular(float deltaTime, const glm::vec3& angular) {
        // Integrate angular velocity to produce a quaternion rotation
        // Using the formula q = (cos(|ω|/2), (ω/|ω|) * sin(|ω|/2)) where ω = Δt * angular
        glm::vec3 omega = angular * deltaTime;
        float magnitude = glm::length(omega);
        
        // For very small rotations, return identity quaternion
        if (magnitude < 0.0001f) {
            return Quaternion(0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
        }
        
        // Normalize omega to get axis
        glm::vec3 axis = omega / magnitude;
        
        // Return quaternion
        return Quaternion(
            glm::degrees(magnitude), // Convert radians to degrees for the angle
            axis
        );
    }

    void integrateAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel) {
        if (!obj) return;
        
        // Update velocity based on acceleration (v += a * dt)
        obj->velocity += accel * deltaTime;
    }

    void integrateAngularAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel) {
        if (!obj) return;
        
        // Update angular velocity based on angular acceleration
        obj->angularVelocity += accel * deltaTime;
    }

    void applyLinearImpulse(GameObject* obj, const glm::vec3& impulse) {
        if (!obj) return;
        
        // Apply impulse directly to velocity
        obj->velocity += impulse;
    }

    void applyAngularImpulse(GameObject* obj, const glm::vec3& impulse) {
        if (!obj) return;
        
        // Apply angular impulse directly to angular velocity
        obj->angularVelocity += impulse;
    }

    void updateObject(GameObject* obj, float deltaTime, bool applyGravity) {
        if (!obj || obj->isStatic) return;
        
        // Apply gravity if requested
        if (applyGravity) {
            integrateAcceleration(obj, deltaTime, gravity);
        }
        
        // Update position based on velocity
        obj->position += integrateLinear(deltaTime, obj->velocity);
        
        // Update rotation based on angular velocity
        if (glm::length(obj->angularVelocity) > 0.0001f) {
            Quaternion rotationDelta = integrateAngular(deltaTime, obj->angularVelocity);
            obj->rotation = rotationDelta * obj->rotation;
        }
        
        // Update the model matrix with new position and rotation
        obj->modelMatrix = glm::translate(glm::mat4(1.0f), obj->position) * 
                           obj->rotation.toMatrix();
    }
    
    void runPhysicsTest() {
        std::cout << "=== Physics Integration Test ===" << std::endl;
        
        // Test 1: Linear integration
        std::cout << "Test 1: Linear Integration" << std::endl;
        glm::vec3 velocity(1.0f, 2.0f, 3.0f);
        float dt = 0.5f;
        glm::vec3 positionChange = integrateLinear(dt, velocity);
        std::cout << "Velocity: " << glm::to_string(velocity) << std::endl;
        std::cout << "Delta Time: " << dt << " seconds" << std::endl;
        std::cout << "Position Change: " << glm::to_string(positionChange) << std::endl;
        std::cout << "Expected: (0.5, 1.0, 1.5)" << std::endl;
        
        // Test 2: Angular integration
        std::cout << "\nTest 2: Angular Integration" << std::endl;
        glm::vec3 angularVelocity(0.0f, 0.0f, 90.0f); // 90 degrees per second around Z
        Quaternion rotationDelta = integrateAngular(dt, angularVelocity);
        std::cout << "Angular Velocity: " << glm::to_string(angularVelocity) << " degrees/s" << std::endl;
        std::cout << "Delta Time: " << dt << " seconds" << std::endl;
        std::cout << "Rotation Delta Angle: " << rotationDelta.getAngle() << " degrees" << std::endl;
        std::cout << "Rotation Delta Axis: " << glm::to_string(rotationDelta.getAxis()) << std::endl;
        std::cout << "Expected Angle: 45 degrees around Z axis" << std::endl;
        
        std::cout << "\nPhysics integration test complete." << std::endl;
    }
}