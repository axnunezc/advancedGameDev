#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include "Shape.hpp"
#include "Quaternion.hpp"
#include <glm/glm.hpp>
#include <functional>

// Forward declaration
class GameObject;

// Forward declarations for physics functions
namespace Physics {
    void updateObject(GameObject* obj, float deltaTime, bool applyGravity);
    void integrateAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel);
    void integrateAngularAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel);
    void applyLinearImpulse(GameObject* obj, const glm::vec3& impulse);
    void applyAngularImpulse(GameObject* obj, const glm::vec3& impulse);
}

class GameObject {
private:
    Shape renderElementShape;
    int renderElement;
    std::function<void(GameObject*, float)> updateFunction;
    
protected:
    glm::vec3 position;
    Quaternion rotation;
    glm::mat4 modelMatrix;
    
    // Add physics state
    glm::vec3 velocity;           // Linear velocity
    glm::vec3 angularVelocity;    // Angular velocity around each axis
    float mass;                   // Mass of the object
    float inverseMass;            // 1/mass, precomputed for efficiency
    bool isStatic;                // Whether the object is affected by physics
    
    // Make physics functions friends to access protected members
    friend void Physics::updateObject(GameObject* obj, float deltaTime, bool applyGravity);
    friend void Physics::integrateAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel);
    friend void Physics::integrateAngularAcceleration(GameObject* obj, float deltaTime, const glm::vec3& accel);
    friend void Physics::applyLinearImpulse(GameObject* obj, const glm::vec3& impulse);
    friend void Physics::applyAngularImpulse(GameObject* obj, const glm::vec3& impulse);
    
public:
    // Constructor
    GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id);
    
    // Virtual destructor
    virtual ~GameObject() = default;
    
    // Core methods
    virtual void update(float deltaTime);
    
    // Getters for rendering
    unsigned int getVAO() const { return renderElementShape.getVAO(); }
    unsigned int getVBO() const { return renderElementShape.getVBO(); }
    int getVertexCount() const { return renderElementShape.getVertexCount(); }
    const glm::mat4& getModelMatrix() const { return modelMatrix; }
    
    // Position setters and getters
    void setPosition(const glm::vec3& pos) { position = pos; }
    const glm::vec3& getPosition() const { return position; }
    
    // Rotation setters and getters
    void setRotation(const Quaternion& rot) { rotation = rot; }
    const Quaternion& getRotation() const { return rotation; }
    
    // Physics setters/getters
    void setVelocity(const glm::vec3& vel) { velocity = vel; }
    const glm::vec3& getVelocity() const { return velocity; }
    
    void setAngularVelocity(const glm::vec3& angVel) { angularVelocity = angVel; }
    const glm::vec3& getAngularVelocity() const { return angularVelocity; }
    
    void setMass(float m) { 
        mass = m; 
        inverseMass = (m > 0.0f) ? 1.0f / m : 0.0f;
    }
    float getMass() const { return mass; }
    float getInverseMass() const { return inverseMass; }
    
    void setStatic(bool staticObj) { isStatic = staticObj; }
    bool getStatic() const { return isStatic; }
    
    // Set a custom update function
    void setUpdateFunction(std::function<void(GameObject*, float)> func) {
        updateFunction = func;
    }
};

#endif // GAMEOBJECT_HPP