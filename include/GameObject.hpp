#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include "Shape.hpp"
#include "Quaternion.hpp"
#include <functional>

class GameObject {
protected:
    glm::vec3 position;
    Quaternion rotation;
    glm::mat4 modelMatrix;
    const Shape& renderElementShape;
    int renderElement;
    
    // Function pointer for update behavior
    std::function<void(GameObject*, float)> updateFunction;
    
public:
    GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id);
    ~GameObject() = default;
    
    // Update method calls the function pointer
    void update(float deltaTime) {
        if (updateFunction) {
            updateFunction(this, deltaTime);
        }
    }
    
    // Method to set the update function
    void setUpdateFunction(std::function<void(GameObject*, float)> updateFn) {
        updateFunction = updateFn;
    }
    
    // Getters for rendering
    const glm::mat4& getModelMatrix() const { return modelMatrix; }
    GLuint getVAO() const { return renderElementShape.getVAO(); }
    GLuint getVBO() const { return renderElementShape.getVBO(); }
    int getVertexCount() const { return renderElementShape.getVertexCount(); }
    
    // Getters and setters for position and rotation
    const glm::vec3& getPosition() const { return position; }
    void setPosition(const glm::vec3& pos) { position = pos; }
    
    const Quaternion& getRotation() const { return rotation; }
    void setRotation(const Quaternion& rot) { rotation = rot; }
};

#endif // GAMEOBJECT_HPP