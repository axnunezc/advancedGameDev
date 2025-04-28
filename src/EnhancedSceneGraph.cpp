#include "EnhancedSceneGraph.hpp"
#include "GJK.hpp"
#include "MPR.hpp"
#include "GameObject.hpp"
#include "AABB.hpp"
#include <algorithm>

// EnhancedSceneGraph implementation

EnhancedSceneGraph::EnhancedSceneGraph(const AABB& worldBounds) 
    : SceneGraph(worldBounds), collisionMethod(GJK) {
}

void EnhancedSceneGraph::setCollisionMethod(CollisionMethod method) {
    collisionMethod = method;
}

void EnhancedSceneGraph::detectCollisions(std::vector<std::pair<GameObject*, GameObject*>>& collisions) {
    if (collisionMethod == AABB_ONLY) {
        // Use base class implementation for AABB-only
        SceneGraph::detectCollisions(collisions);
        return;
    }
    
    // Get all objects in the scene
    std::vector<GameObject*> allObjects;
    
    // Helper function to collect all objects from scene hierarchy
    std::function<void(SceneNode*)> collectObjects = [&](SceneNode* node) {
        // Add objects from this node
        for (auto* obj : node->getObjects()) {
            allObjects.push_back(obj);
        }
        
        // Process children
        for (const auto& child : node->children) {
            collectObjects(child.get());
        }
    };
    
    collectObjects(getRootNode());
    
    // Use octree for broad-phase collision detection
    for (auto* obj : allObjects) {
        std::vector<GameObject*> potentialCollisions;
        // Use base class method for broad phase
        SceneGraph::detectCollisions(obj, potentialCollisions);
        
        // For each potential collision, perform narrow-phase check
        for (auto* other : potentialCollisions) {
            // Avoid duplicate pairs
            if (obj < other) {
                // Perform narrow-phase collision detection
                bool isColliding = false;
                
                if (collisionMethod == GJK) {
                    // Use the collision flag from GJKResult
                    GJKResult result = Collision::GJK(
                        obj->getShape(), obj->getRotation(), obj->getPosition(),
                        other->getShape(), other->getRotation(), other->getPosition()
                    );
                    isColliding = result.collision;
                }
                else if (collisionMethod == MPR) {
                    isColliding = Collision::MPR(
                        obj->getShape(), obj->getRotation(), obj->getPosition(),
                        other->getShape(), other->getRotation(), other->getPosition()
                    );
                }
                
                if (isColliding) {
                    collisions.push_back(std::make_pair(obj, other));
                }
            }
        }
    }
}

void EnhancedSceneGraph::detectCollisions(GameObject* obj, std::vector<GameObject*>& collidingObjects) {
    if (collisionMethod == AABB_ONLY) {
        // Use base class implementation for AABB-only
        SceneGraph::detectCollisions(obj, collidingObjects);
        return;
    }
    
    // Get potential collisions using octree (broad phase)
    std::vector<GameObject*> potentialCollisions;
    SceneGraph::detectCollisions(obj, potentialCollisions);
    
    // Clear output vector
    collidingObjects.clear();
    
    // Perform narrow-phase collision detection
    for (auto* other : potentialCollisions) {
        bool isColliding = false;
        
        if (collisionMethod == GJK) {
            // Use the collision flag from GJKResult
            GJKResult result = Collision::GJK(
                obj->getShape(), obj->getRotation(), obj->getPosition(),
                other->getShape(), other->getRotation(), other->getPosition()
            );
            isColliding = result.collision;
        }
        else if (collisionMethod == MPR) {
            isColliding = Collision::MPR(
                obj->getShape(), obj->getRotation(), obj->getPosition(),
                other->getShape(), other->getRotation(), other->getPosition()
            );
        }
        
        if (isColliding) {
            collidingObjects.push_back(other);
        }
    }
}

void EnhancedSceneGraph::processCollisionResponses() {
    std::vector<std::pair<GameObject*, GameObject*>> collisions;
    detectCollisions(collisions);
    
    for (const auto& collision : collisions) {
        enhancedResponder.processCollision(collision.first, collision.second);
    }
}

EnhancedCollisionResponder& EnhancedSceneGraph::getResponder() {
    return enhancedResponder;
}

EnhancedSceneGraph::CollisionMethod EnhancedSceneGraph::getCollisionMethod() const {
    return collisionMethod;
}

// EnhancedCollisionResponder implementation

EnhancedCollisionResponder::EnhancedCollisionResponder() 
    : method(GJK) {
}

void EnhancedCollisionResponder::setMethod(DetectionMethod newMethod) {
    method = newMethod;
}

void EnhancedCollisionResponder::processCollision(GameObject* objA, GameObject* objB) {
    int typeA = objA->getTypeId();
    int typeB = objB->getTypeId();
    
    // Always do AABB test first
    if (!objA->getBoundingBox().overlaps(objB->getBoundingBox())) {
        return;
    }
    
    // If we're just using AABBs, call the callback now
    if (method == AABB_ONLY) {
        int index = getIndex(typeA, typeB);
        if (index < callbackTable.size() && callbackTable[index]) {
            callbackTable[index](objA, objB);
        }
        return;
    }
    
    // Perform narrow-phase collision detection
    bool collision = false;
    
    if (method == GJK) {
        // Use the collision flag from GJKResult
        GJKResult result = Collision::GJK(
            objA->getShape(), objA->getRotation(), objA->getPosition(),
            objB->getShape(), objB->getRotation(), objB->getPosition()
        );
        collision = result.collision;
    } else if (method == MPR) {
        collision = Collision::MPR(
            objA->getShape(), objA->getRotation(), objA->getPosition(),
            objB->getShape(), objB->getRotation(), objB->getPosition()
        );
    }
    
    // If collision detected, call the callback
    if (collision) {
        int index = getIndex(typeA, typeB);
        if (index < callbackTable.size() && callbackTable[index]) {
            callbackTable[index](objA, objB);
        }
    }
}