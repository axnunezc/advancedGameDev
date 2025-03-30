#include "GameObject.hpp"
#include <glm/gtc/matrix_transform.hpp>

GameObject::GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id)
    : position(pos),
      rotation(rot),
      renderElementShape(shape),
      renderElement(id),
      updateFunction(nullptr),
      velocity(0.0f),                 // Initialize physics properties
      angularVelocity(0.0f),
      mass(1.0f),
      inverseMass(1.0f),
      isStatic(false)
{
    // Construct the model matrix using the position and rotation values
    modelMatrix = glm::translate(glm::mat4(1.0f), position) *
                  rotation.toMatrix();
}

void GameObject::update(float deltaTime) {
    // Use custom update function if available
    if (updateFunction) {
        updateFunction(this, deltaTime);
        return;
    }
    
    // No custom update - do nothing by default
    // Physics integration happens in PhysicsIntegrator::updateObject
}