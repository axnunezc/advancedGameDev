#include "GameObject.hpp"

GameObject::GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id)
    : position(pos), rotation(rot), renderElementShape(shape), renderElement(id),
      linearVelocity(glm::vec3(0.0f)), angularVelocity(glm::vec3(0.0f)) {
        
        // Construct the model matrix using the actual position and rotation values
        modelMatrix = glm::translate(glm::mat4(1.0f), position) *
                      glm::rotate(glm::mat4(1.0f), glm::radians(rot.getAngle()), rot.getAxis());
}

void GameObject::update(float deltaTime) {
    float angle = 90.0f * deltaTime; // Rotate 90 degrees per second

    // Apply rotation incrementally using glm::rotate
    modelMatrix = glm::translate(glm::mat4(1.0f), position) * 
                    glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
}
