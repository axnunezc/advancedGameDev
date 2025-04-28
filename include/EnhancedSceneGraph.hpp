#ifndef ENHANCED_SCENE_GRAPH_HPP
#define ENHANCED_SCENE_GRAPH_HPP

#include "SceneGraph.hpp"
#include <functional>

// Forward declarations
class GameObject;
class AABB;

// Enhanced CollisionResponder with GJK/MPR support
class EnhancedCollisionResponder : public CollisionResponder {
public:
    enum DetectionMethod {
        AABB_ONLY,  // Just use bounding boxes (fast but imprecise)
        GJK,        // Use GJK for narrow phase (precise)
        MPR         // Use MPR for narrow phase (precise, better for penetration)
    };
    
    EnhancedCollisionResponder();
    
    void setMethod(DetectionMethod newMethod);
    
    // Enhanced collision processing
    void processCollision(GameObject* objA, GameObject* objB);
    
private:
    DetectionMethod method;
};

// Enhanced SceneGraph with GJK/MPR collision detection
class EnhancedSceneGraph : public SceneGraph {
public:
    enum CollisionMethod {
        AABB_ONLY,  // Fast but less accurate
        GJK,        // Accurate collision detection
        MPR         // Better for penetration detection
    };
    
    EnhancedSceneGraph(const AABB& worldBounds);
    
    void setCollisionMethod(CollisionMethod method);
    
    // Override the collision detection methods without using override keyword
    void detectCollisions(std::vector<std::pair<GameObject*, GameObject*>>& collisions);
    void detectCollisions(GameObject* obj, std::vector<GameObject*>& collidingObjects);
    
    // Override the collision response processing without using override keyword
    void processCollisionResponses();

    // Return enhanced responder instead of base class
    EnhancedCollisionResponder& getResponder();
    
    // Method to get the current collision method
    CollisionMethod getCollisionMethod() const;
    
private:
    CollisionMethod collisionMethod;
    EnhancedCollisionResponder enhancedResponder;
};

#endif // ENHANCED_SCENE_GRAPH_HPP