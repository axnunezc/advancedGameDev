#ifndef GAMEOBJECT_MANAGER_HPP
#define GAMEOBJECT_MANAGER_HPP

#include "GameObject.hpp"
#include <iostream>

const size_t MAX_OBJECTS = 100;

class GameObjectManager {
private:
    GameObject* objects[MAX_OBJECTS];
    size_t objectCount;

public:
    GameObjectManager();
    ~GameObjectManager();

    void addObject(GameObject* obj);
    void updateAll(float dt);
    void removeObject(size_t index);

    size_t getCount() const;
    GameObject* getObject(size_t index);
};

#endif
