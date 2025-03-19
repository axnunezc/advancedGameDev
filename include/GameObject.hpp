#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include "Quaternion.hpp"
#include "Shape.hpp"

class GameObject {
protected:
    glm::vec3 position;
    Quaternion rotation;
    glm::mat4 modelMatrix;

private:
    Shape renderElementShape;
    glm::vec3 linearVelocity;
    glm::vec3 angularVelocity;
    int renderElement; 

public:
    GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id);

    virtual ~GameObject() = default;

    // Virtual function for update logic
    virtual void update(float deltaTime) = 0;
    glm::mat4 getModelMatrix() const { return modelMatrix; }

    int getRenderElement() const { return renderElement; }

    GLuint getVAO() const { return renderElementShape.getVAO(); }
    GLuint getVBO() const { return renderElementShape.getVBO(); }
    size_t getVertexCount() const { return renderElementShape.getVertexCount(); }
};

#endif
