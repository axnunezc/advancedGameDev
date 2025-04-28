#ifndef SCENEGRAPH_HPP
#define SCENEGRAPH_HPP

#include "CollisionResponder.hpp"
#include "GameObject.hpp"
#include "AABB.hpp"
#include "Camera.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

// Forward declarations
class Renderer;
class Frustum;
class SceneNode;
class OctreeNode;

// Frustum class for view frustum culling
class Frustum {
public:
    enum PlaneID {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        PlaneCount
    };
    
    glm::vec4 planes[PlaneCount];
    
    void updateFromCamera(const Camera& camera);
    bool containsPoint(const glm::vec3& point) const;
    bool containsSphere(const glm::vec3& center, float radius) const;
    bool containsAABB(const AABB& aabb) const;
};

// Scene node class
class SceneNode {
public:
    SceneNode();
    
    // Transform functions
    void setLocalTransform(const glm::mat4& transform);
    const glm::mat4& getLocalTransform() const;
    const glm::mat4& getWorldTransform();
    void updateWorldTransform();
    
    // Bounds functions
    void updateWorldBounds();
    const AABB& getWorldBounds();
    void setLocalBounds(const AABB& bounds);
    const AABB& getLocalBounds() const;
    
    // Hierarchy functions
    void addChild(std::unique_ptr<SceneNode> child);
    std::unique_ptr<SceneNode> removeChild(SceneNode* child);
    void setParent(SceneNode* parent);
    SceneNode* getParent() const;
    
    // Object functions
    void attachObject(GameObject* obj);
    void detachObject(GameObject* obj);
    const std::vector<GameObject*>& getObjects() const;
    
    // Rendering functions
    void render(Renderer& renderer, const Frustum& frustum);
    void update(float deltaTime);
    void collectVisibleObjects(std::vector<GameObject*>& visibleObjects, const Frustum& frustum);
    
    // Child nodes
    std::vector<std::unique_ptr<SceneNode>> children;
    
private:
    SceneNode* parent;
    
    glm::mat4 localTransform;
    glm::mat4 worldTransform;
    bool transformDirty;
    
    AABB localBounds;
    AABB worldBounds;
    bool boundsDirty;
    
    std::vector<GameObject*> objects;
};

// Octree node for spatial partitioning
class OctreeNode {
public:
    OctreeNode(const AABB& bounds, int maxDepth = 8, int maxObjects = 10, int currentDepth = 0, OctreeNode* parent = nullptr);
    
    void split();
    int getOctantForPoint(const glm::vec3& point) const;
    
    void insert(GameObject* obj);
    void remove(GameObject* obj);
    void update(GameObject* obj);
    
    void collectVisibleObjects(std::vector<GameObject*>& visibleObjects, const Frustum& frustum);
    void clear();
    
    const AABB& getBounds() const;
    bool isLeaf() const { return leafNode; }
    OctreeNode* getChild(int index) const { return children[index].get(); }
    const std::vector<GameObject*>& getObjects() const { return objects; }
    void OctreeNode_findPotentialCollisions(OctreeNode* node, GameObject* obj, std::vector<GameObject*>& potentialCollisions);
    
private:
    AABB bounds;
    int maxDepth;
    int currentDepth;
    int maxObjectsPerNode;
    bool leafNode;
    
    OctreeNode* parent;
    std::unique_ptr<OctreeNode> children[8];
    
    std::vector<GameObject*> objects;
};

// Main scene graph class
class SceneGraph {
public:
    SceneGraph(const AABB& worldBounds = AABB(glm::vec3(-100.0f), glm::vec3(100.0f)));
    
    void addObject(GameObject* obj, SceneNode* parent = nullptr);
    void removeObject(GameObject* obj);
    
    void updateTransforms();
    
    // Update the spatial structure (rebuild octree)
    void updateSpatialStructure();
    
    // Update all objects in the scene
    void updateSpatialStructure(float deltaTime);
    
    // Update a specific object in the scene
    void updateObject(GameObject* obj);
    
    void render(Renderer& renderer, const Camera& camera);
    
    SceneNode* createNode(SceneNode* parent = nullptr);
    void destroyNode(SceneNode* node);
    
    SceneNode* getRootNode() const;
    
    void getVisibleObjects(std::vector<GameObject*>& visibleObjects, const Camera& camera);
    
    // Collision detection methods
    void detectCollisions(std::vector<std::pair<GameObject*, GameObject*>>& collisions);
    void detectCollisions(GameObject* obj, std::vector<GameObject*>& collidingObjects);
    void registerCollisionCallback(int typeA, int typeB, CollisionCallback callback);
    void processCollisionResponses();
    
private:
    std::unique_ptr<SceneNode> rootNode;
    std::unique_ptr<OctreeNode> octreeRoot;
    AABB worldBounds;
    CollisionResponder collisionResponder;
};

#endif // SCENEGRAPH_HPP