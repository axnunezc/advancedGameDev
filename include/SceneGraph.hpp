#ifndef SCENEGRAPH_HPP
#define SCENEGRAPH_HPP

// Enable experimental GLM features
#define GLM_ENABLE_EXPERIMENTAL

#include "GameObject.hpp"
#include "Camera.hpp"  // Include the full Camera definition
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <functional>

// Forward declaration for Renderer (if not needed in header)
class Renderer;

// Axis-Aligned Bounding Box
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    
    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
    
    bool contains(const glm::vec3& point) const {
        return (point.x >= min.x && point.x <= max.x &&
                point.y >= min.y && point.y <= max.y &&
                point.z >= min.z && point.z <= max.z);
    }
    
    bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x &&
                min.y <= other.max.y && max.y >= other.min.y &&
                min.z <= other.max.z && max.z >= other.min.z);
    }
    
    glm::vec3 getCenter() const {
        return (min + max) * 0.5f;
    }
    
    glm::vec3 getExtents() const {
        return (max - min) * 0.5f;
    }
};

// Frustum for view culling
class Frustum {
private:
    enum Planes {
        Near,
        Far,
        Left,
        Right,
        Top,
        Bottom,
        PlaneCount
    };
    
    glm::vec4 planes[PlaneCount]; // Ax + By + Cz + D = 0
    
public:
    Frustum() {}
    
    void updateFromCamera(const Camera& camera);
    
    bool containsPoint(const glm::vec3& point) const;
    bool containsSphere(const glm::vec3& center, float radius) const;
    bool containsAABB(const AABB& aabb) const;
};

// Scene Node for hierarchical transformations
class SceneNode {
private:
    SceneNode* parent;
    std::vector<GameObject*> objects;
    
    glm::mat4 localTransform;
    glm::mat4 worldTransform;
    bool transformDirty;
    
    AABB localBounds;
    AABB worldBounds;
    bool boundsDirty;
    
public:
    std::vector<std::unique_ptr<SceneNode>> children; // Made public for convenience
    
    SceneNode();
    ~SceneNode() = default;
    
    // Transform operations
    void setLocalTransform(const glm::mat4& transform);
    const glm::mat4& getLocalTransform() const;
    const glm::mat4& getWorldTransform();
    void updateWorldTransform();
    
    // Bounds operations
    void updateWorldBounds();
    const AABB& getWorldBounds();
    void setLocalBounds(const AABB& bounds);
    const AABB& getLocalBounds() const;
    
    // Hierarchy management
    void addChild(std::unique_ptr<SceneNode> child);
    std::unique_ptr<SceneNode> removeChild(SceneNode* child);
    void setParent(SceneNode* parent);
    SceneNode* getParent() const;
    
    // Object management
    void attachObject(GameObject* obj);
    void detachObject(GameObject* obj);
    const std::vector<GameObject*>& getObjects() const;
    
    // Scene traversal
    void render(Renderer& renderer, const Frustum& frustum);
    void update(float deltaTime);
    void collectVisibleObjects(std::vector<GameObject*>& visibleObjects, const Frustum& frustum);
};

// Octree node for spatial partitioning
class OctreeNode {
private:
    AABB bounds;
    std::unique_ptr<OctreeNode> children[8];
    std::vector<GameObject*> objects;
    int maxDepth;
    int currentDepth;
    int maxObjectsPerNode;
    bool isLeaf;
    OctreeNode* parent;  // Added parent pointer
    
    void split();
    int getOctantForPoint(const glm::vec3& point) const;
    
public:
    OctreeNode(const AABB& bounds, int maxDepth = 8, int maxObjects = 16, int currentDepth = 0, OctreeNode* parent = nullptr);
    ~OctreeNode() = default;
    
    void insert(GameObject* obj);
    void remove(GameObject* obj);
    void update(GameObject* obj);
    
    void collectVisibleObjects(std::vector<GameObject*>& visibleObjects, const Frustum& frustum);
    void clear();
    
    const AABB& getBounds() const;
};

// Main Scene Graph class
class SceneGraph {
private:
    std::unique_ptr<SceneNode> rootNode;
    std::unique_ptr<OctreeNode> octreeRoot;
    AABB worldBounds;
    
public:
    SceneGraph(const AABB& worldBounds);
    ~SceneGraph() = default;
    
    // Scene operations
    void addObject(GameObject* obj, SceneNode* parent = nullptr);
    void removeObject(GameObject* obj);
    void updateTransforms();
    
    // Rendering
    void render(Renderer& renderer, const Camera& camera);
    
    // Scene management
    SceneNode* createNode(SceneNode* parent = nullptr);
    void destroyNode(SceneNode* node);
    SceneNode* getRootNode() const;
    
    // Queries
    void getVisibleObjects(std::vector<GameObject*>& visibleObjects, const Camera& camera);
    
    // Spatial update
    void updateSpatialStructure();
};

#endif // SCENEGRAPH_HPP