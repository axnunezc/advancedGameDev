#include "GameObject.hpp"
#include <glm/gtc/matrix_transform.hpp>

GameObject::GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id)
    : position(pos),
      rotation(rot),
      renderElementShape(shape),
      renderElement(id),
      updateFunction(nullptr) // Initialize with no update function
{
    // Construct the model matrix using the position and rotation values
    modelMatrix = glm::translate(glm::mat4(1.0f), position) *
                  rotation.toMatrix();
}