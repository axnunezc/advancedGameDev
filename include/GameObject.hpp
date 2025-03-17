#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP

#include "Quat.hpp"
#include "Shape.hpp"
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class GameObject {
protected:
    glm::vec3 position;
    Quat rotation;
    Shape* shape;
    glm::vec3 linearV;
    glm::vec3 angularV;

public:
    std::function<void(GameObject*, float)> updateFunc;

    GameObject(const glm::vec3& pos, const Quat& rot, Shape* mesh);
    virtual ~GameObject();

    virtual void update(float dt) = 0;
    void updateViaFunction(float dt);
    glm::mat4 getModel() const;
    void integrateVelocity(float dt);
    void draw() const;
    void setAngularVelocity(const glm::vec3& velocity);

    virtual Quat getRotation() const { return rotation; }
    virtual void setRotation(const Quat& rot) { rotation = rot; }
};

class Cube : public GameObject {
public:
    Cube(const glm::vec3& pos, const Quat& rot, Shape* mesh);
    void update(float dt) override;
};

void cubeUpdate(GameObject* obj, float dt);

class Cube_FnPtr : public GameObject {
public:
    void (*updateFn)(GameObject*, float);  // Function pointer for update behavior

    Cube_FnPtr(const glm::vec3& pos, const Quat& rot, Shape* shape)
        : GameObject(pos, rot, shape), updateFn(nullptr) {}

    void setUpdateFunction(void (*fn)(GameObject*, float)) {
        updateFn = fn;
    }

    void update(float dt) override {
        if (updateFn) updateFn(this, dt);
    }
};

void cubeUpdateFunction(GameObject* obj, float dt);

#endif
