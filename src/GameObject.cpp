#include "GameObject.hpp"

GameObject::GameObject(const glm::vec3& pos, const Quat& rot, Shape* mesh)
    : position(pos), rotation(rot), shape(mesh), linearV(0.0f), angularV(0.0f), updateFunc(nullptr) {}

GameObject::~GameObject() {
    delete shape;
}

void GameObject::updateViaFunction(float dt) {
    if (updateFunc) updateFunc(this, dt);
}

glm::mat4 GameObject::getModel() const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model *= rotation.toMat4();
    return model;
}

void GameObject::integrateVelocity(float dt) {
    position += linearV * dt;
    rotation = rotation * Quat(0, angularV.x * dt, angularV.y * dt, angularV.z * dt);
}

void GameObject::draw() const {
    if (shape) {
        shape->bind();
        glDrawArrays(GL_TRIANGLES, 0, shape->getVertexCount());
        shape->unbind();
    }
}

void GameObject::setAngularVelocity(const glm::vec3& velocity) {
    angularV = velocity;
}

// ✅ Cube Constructor and Update Method
Cube::Cube(const glm::vec3& pos, const Quat& rot, Shape* mesh)
    : GameObject(pos, rot, mesh) {}

void Cube::update(float dt) {
    float rotationSpeed = glm::radians(45.0f);
    setAngularVelocity(glm::vec3(0.0f, rotationSpeed, 0.0f));
    integrateVelocity(dt);
}

// ✅ Function Pointer Update for Cube
void cubeUpdate(GameObject* obj, float dt) {
    float rotationSpeed = glm::radians(45.0f);
    obj->setAngularVelocity(glm::vec3(0.0f, rotationSpeed, 0.0f));
    obj->integrateVelocity(dt);
}

void cubeUpdateFunction(GameObject* obj, float dt) {
    Cube_FnPtr* cube = dynamic_cast<Cube_FnPtr*>(obj);
    if (cube) {
        cube->setRotation(cube->getRotation() * Quat(glm::vec3(0, 1, 0), dt));
    }
}
