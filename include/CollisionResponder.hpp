#ifndef COLLISIONRESPONDER_HPP
#define COLLISIONRESPONDER_HPP

#include "GameObject.hpp"
#include <functional>
#include <vector>

// Forward declare TypeRegistry
class TypeRegistry;

// Define the callback function type
using CollisionCallback = std::function<void(GameObject*, GameObject*)>;

class CollisionResponder {
protected:
    // Store callbacks in a triangular 2D array (only need to store half since collisions are symmetric)
    std::vector<CollisionCallback> callbackTable;
    int numTypes;

    // Convert 2D coordinates to 1D index in the triangular array
    int getIndex(int typeA, int typeB);

public:
    CollisionResponder();

    // Resize the callback table when new types are registered
    void resize();

    // Register a collision callback between two GameObjects
    void registerCallback(int typeA, int typeB, CollisionCallback callback);

    // Process a collision between two GameObjects
    void processCollision(GameObject* objA, GameObject* objB);
};

#endif // COLLISIONRESPONDER_HPP