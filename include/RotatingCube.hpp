#ifndef ROTATINGCUBE_HPP
#define ROTATINGCUBE_HPP

#include "GameObject.hpp"

class RotatingCube : public GameObject {
private:
    glm::vec3 rotationAxis;
    float rotationSpeed;

public:
    RotatingCube(glm::vec3 pos, Quaternion rot, const Shape& shape, int id, glm::vec3 axis, float speed)
        : GameObject(pos, rot, shape, id), rotationAxis(glm::normalize(axis)), rotationSpeed(speed) 
    {
        // Set custom update function for this type
        setUpdateFunction(&RotatingCube::customUpdate);
    }

    // Static update function for RotatingCube objects
    static void customUpdate(GameObject* obj, float deltaTime) {
        // Need to cast to RotatingCube to access specific members
        RotatingCube* cube = static_cast<RotatingCube*>(obj);
        
        // Apply rotation based on rotationAxis and rotationSpeed
        float angle = cube->rotationSpeed * deltaTime;
        
        cube->modelMatrix = glm::translate(glm::mat4(1.0f), cube->position) *
                           glm::rotate(glm::mat4(1.0f), glm::radians(angle), cube->rotationAxis);
    }
};

#endif