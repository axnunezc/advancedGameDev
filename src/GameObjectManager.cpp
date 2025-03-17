#include "GameObjectManager.hpp"

GameObjectManager::GameObjectManager() : objectCount(0) {}

void GameObjectManager::addObject(GameObject* obj) {
    if (objectCount < MAX_OBJECTS) {
        objects[objectCount++] = obj;
    } else {
        std::cerr << "Max GameObject limit reached!\n";
    }
}

void GameObjectManager::updateAll(float dt) {
    for (size_t i = 0; i < objectCount; ++i) {
        objects[i]->update(dt);
    }
}

void GameObjectManager::removeObject(size_t index) {
    if (index < objectCount) {
        delete objects[index];
        objects[index] = objects[objectCount - 1];
        objectCount--;
    }
}

size_t GameObjectManager::getCount() const { return objectCount; }
GameObject* GameObjectManager::getObject(size_t index) { return (index < objectCount) ? objects[index] : nullptr; }

GameObjectManager::~GameObjectManager() {
    for (size_t i = 0; i < objectCount; ++i) {
        delete objects[i];
    }
}
