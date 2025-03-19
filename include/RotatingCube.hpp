#ifndef ROTATINGCUBE_HPP
#define ROTATINGCUBE_HPP

#include "GameObject.hpp"

class RotatingCube : public GameObject {
private:
    glm::vec3 rotationAxis;
    float rotationSpeed;

public:
    RotatingCube(glm::vec3 pos, Quaternion rot, const Shape& shape, int id, glm::vec3 axis, float speed)
        : GameObject(pos, rot, shape, id), rotationAxis(glm::normalize(axis)), rotationSpeed(speed) {}

    void update(float deltaTime) override {
        // // Accumulate rotation
        // float angleIncrement = deltaTime * 60.0f;  // Rotate based on elapsed time
        // rotation = Quaternion(angleIncrement, rotation.getAxis()) * rotation; // Apply incremental rotation
        // rotation.normalize(); // Keep it unit length

        // // Update the model matrix with the new rotation
        // modelMatrix = glm::translate(glm::mat4(1.0f), position) *
        //             glm::rotate(glm::mat4(1.0f), glm::radians(rotation.getAngle()), rotation.getAxis());
    }
};

#endif
