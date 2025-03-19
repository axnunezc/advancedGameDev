#include "SceneGraph.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

// ----- Frustum Implementation -----

void Frustum::updateFromCamera(const Camera& camera) {
    // Extract planes from view-projection matrix
    glm::mat4 viewProj = camera.getProjectionMatrix() * camera.getViewMatrix();
    
    // Left plane
    planes[Left].x = viewProj[0][3] + viewProj[0][0];
    planes[Left].y = viewProj[1][3] + viewProj[1][0];
    planes[Left].z = viewProj[2][3] + viewProj[2][0];
    planes[Left].w = viewProj[3][3] + viewProj[3][0];
    
    // Right plane
    planes[Right].x = viewProj[0][3] - viewProj[0][0];
    planes[Right].y = viewProj[1][3] - viewProj[1][0];
    planes[Right].z = viewProj[2][3] - viewProj[2][0];
    planes[Right].w = viewProj[3][3] - viewProj[3][0];
    
    // Bottom plane
    planes[Bottom].x = viewProj[0][3] + viewProj[0][1];
    planes[Bottom].y = viewProj[1][3] + viewProj[1][1];
    planes[Bottom].z = viewProj[2][3] + viewProj[2][1];
    planes[Bottom].w = viewProj[3][3] + viewProj[3][1];
    
    // Top plane
    planes[Top].x = viewProj[0][3] - viewProj[0][1];
    planes[Top].y = viewProj[1][3] - viewProj[1][1];
    planes[Top].z = viewProj[2][3] - viewProj[2][1];
    planes[Top].w = viewProj[3][3] - viewProj[3][1];
    
    // Near plane
    planes[Near].x = viewProj[0][3] + viewProj[0][2];
    planes[Near].y = viewProj[1][3] + viewProj[1][2];
    planes[Near].z = viewProj[2][3] + viewProj[2][2];
    planes[Near].w = viewProj[3][3] + viewProj[3][2];
    
    // Far plane
    planes[Far].x = viewProj[0][3] - viewProj[0][2];
    planes[Far].y = viewProj[1][3] - viewProj[1][2];
    planes[Far].z = viewProj[2][3] - viewProj[2][2];
    planes[Far].w = viewProj[3][3] - viewProj[3][2];
    
    // Normalize planes
    for (int i = 0; i < PlaneCount; ++i) {
        float length = sqrtf(planes[i].x * planes[i].x + 
                            planes[i].y * planes[i].y + 
                            planes[i].z * planes[i].z);
        planes[i] /= length;
    }
}

bool Frustum::containsPoint(const glm::vec3& point) const {
    for (int i = 0; i < PlaneCount; ++i) {
        if (planes[i].x * point.x + planes[i].y * point.y + planes[i].z * point.z + planes[i].w <= 0) {
            return false;
        }
    }
    return true;
}

bool Frustum::containsSphere(const glm::vec3& center, float radius) const {
    for (int i = 0; i < PlaneCount; ++i) {
        float distance = planes[i].x * center.x + 
                         planes[i].y * center.y + 
                         planes[i].z * center.z + 
                         planes[i].w;
        if (distance <= -radius) {
            return false;
        }
    }
    return true;
}

bool Frustum::containsAABB(const AABB& aabb) const {
    // Check if any point of the box is inside the frustum
    glm::vec3 corners[8];
    corners[0] = glm::vec3(aabb.min.x, aabb.min.y, aabb.min.z);
    corners[1] = glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z);
    corners[2] = glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z);
    corners[3] = glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z);
    corners[4] = glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z);
    corners[5] = glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z);
    corners[6] = glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z);
    corners[7] = glm::vec3(aabb.max.x, aabb.max.y, aabb.max.z);
    
    // For each plane, check if all corners are on the negative side
    for (int i = 0; i < PlaneCount; ++i) {
        bool allOutside = true;
        for (int j = 0; j < 8; ++j) {
            if (planes[i].x * corners[j].x + 
                planes[i].y * corners[j].y + 
                planes[i].z * corners[j].z + 
                planes[i].w > 0) {
                allOutside = false;
                break;
            }
        }
        if (allOutside) {
            return false;
        }
    }
    
    return true;
}

// ----- SceneNode Implementation -----

SceneNode::SceneNode() 
    : parent(nullptr), 
      localTransform(1.0f), 
      worldTransform(1.0f),
      transformDirty(false),
      localBounds(glm::vec3(-1.0f), glm::vec3(1.0f)),
      worldBounds(glm::vec3(-1.0f), glm::vec3(1.0f)),
      boundsDirty(true) {
}

void SceneNode::setLocalTransform(const glm::mat4& transform) {
    localTransform = transform;
    transformDirty = true;
    boundsDirty = true;
    
    // Mark all child transforms as dirty
    for (auto& child : children) {
        child->transformDirty = true;
        child->boundsDirty = true;
    }
}

const glm::mat4& SceneNode::getLocalTransform() const {
    return localTransform;
}

const glm::mat4& SceneNode::getWorldTransform() {
    if (transformDirty) {
        updateWorldTransform();
    }
    return worldTransform;
}

void SceneNode::updateWorldTransform() {
    if (parent) {
        worldTransform = parent->getWorldTransform() * localTransform;
    } else {
        worldTransform = localTransform;
    }
    
    transformDirty = false;
    boundsDirty = true;
    
    // Update child transforms
    for (auto& child : children) {
        child->updateWorldTransform();
    }
}

void SceneNode::updateWorldBounds() {
    if (!boundsDirty) {
        return;
    }
    
    // Start with local bounds transformed to world space
    glm::vec3 corners[8];
    corners[0] = glm::vec3(localBounds.min.x, localBounds.min.y, localBounds.min.z);
    corners[1] = glm::vec3(localBounds.min.x, localBounds.min.y, localBounds.max.z);
    corners[2] = glm::vec3(localBounds.min.x, localBounds.max.y, localBounds.min.z);
    corners[3] = glm::vec3(localBounds.min.x, localBounds.max.y, localBounds.max.z);
    corners[4] = glm::vec3(localBounds.max.x, localBounds.min.y, localBounds.min.z);
    corners[5] = glm::vec3(localBounds.max.x, localBounds.min.y, localBounds.max.z);
    corners[6] = glm::vec3(localBounds.max.x, localBounds.max.y, localBounds.min.z);
    corners[7] = glm::vec3(localBounds.max.x, localBounds.max.y, localBounds.max.z);
    
    // Transform corners to world space
    const glm::mat4& transform = getWorldTransform();
    for (int i = 0; i < 8; ++i) {
        glm::vec4 transformedCorner = transform * glm::vec4(corners[i], 1.0f);
        corners[i] = glm::vec3(transformedCorner) / transformedCorner.w;
    }
    
    // Find min/max of transformed corners
    glm::vec3 min = corners[0];
    glm::vec3 max = corners[0];
    
    for (int i = 1; i < 8; ++i) {
        min = glm::min(min, corners[i]);
        max = glm::max(max, corners[i]);
    }
    
    worldBounds.min = min;
    worldBounds.max = max;
    
    // Include child bounds
    for (auto& child : children) {
        child->updateWorldBounds();
        const AABB& childBounds = child->getWorldBounds();
        worldBounds.min = glm::min(worldBounds.min, childBounds.min);
        worldBounds.max = glm::max(worldBounds.max, childBounds.max);
    }
    
    // Include attached objects
    for (auto* obj : objects) {
        // Extract position from model matrix instead of using getPosition()
        glm::mat4 objMatrix = obj->getModelMatrix();
        glm::vec3 objPos(objMatrix[3][0], objMatrix[3][1], objMatrix[3][2]);
        
        float objRadius = 1.0f; // Use a default radius or calculate from shape if available
        
        worldBounds.min = glm::min(worldBounds.min, objPos - glm::vec3(objRadius));
        worldBounds.max = glm::max(worldBounds.max, objPos + glm::vec3(objRadius));
    }
    
    boundsDirty = false;
}

const AABB& SceneNode::getWorldBounds() {
    if (boundsDirty) {
        updateWorldBounds();
    }
    return worldBounds;
}

void SceneNode::setLocalBounds(const AABB& bounds) {
    localBounds = bounds;
    boundsDirty = true;
}

const AABB& SceneNode::getLocalBounds() const {
    return localBounds;
}

void SceneNode::addChild(std::unique_ptr<SceneNode> child) {
    child->parent = this;
    child->transformDirty = true;
    child->boundsDirty = true;
    children.push_back(std::move(child));
}

std::unique_ptr<SceneNode> SceneNode::removeChild(SceneNode* child) {
    auto it = std::find_if(children.begin(), children.end(),
                          [child](const std::unique_ptr<SceneNode>& node) {
                              return node.get() == child;
                          });
    
    if (it != children.end()) {
        std::unique_ptr<SceneNode> result = std::move(*it);
        result->parent = nullptr;
        children.erase(it);
        return result;
    }
    
    return nullptr;
}

void SceneNode::setParent(SceneNode* newParent) {
    if (parent) {
        parent->removeChild(this);
    }
    
    parent = newParent;
    transformDirty = true;
    boundsDirty = true;
}

SceneNode* SceneNode::getParent() const {
    return parent;
}

void SceneNode::attachObject(GameObject* obj) {
    objects.push_back(obj);
    boundsDirty = true;
}

void SceneNode::detachObject(GameObject* obj) {
    auto it = std::find(objects.begin(), objects.end(), obj);
    if (it != objects.end()) {
        objects.erase(it);
        boundsDirty = true;
    }
}

const std::vector<GameObject*>& SceneNode::getObjects() const {
    return objects;
}

void SceneNode::render(Renderer& renderer, const Frustum& frustum) {
    // Check if node is visible
    if (!frustum.containsAABB(getWorldBounds())) {
        return;
    }
    
    // Render all attached objects
    for (auto* obj : objects) {
        renderer.submit(obj);
    }
    
    // Render all children
    for (auto& child : children) {
        child->render(renderer, frustum);
    }
}

void SceneNode::update(float deltaTime) {
    // Update attached objects
    for (auto* obj : objects) {
        obj->update(deltaTime);
    }
    
    // Update children
    for (auto& child : children) {
        child->update(deltaTime);
    }
}

void SceneNode::collectVisibleObjects(std::vector<GameObject*>& visibleObjects, const Frustum& frustum) {
    // Check if node is visible
    if (!frustum.containsAABB(getWorldBounds())) {
        return;
    }
    
    // Add all attached objects
    for (auto* obj : objects) {
        visibleObjects.push_back(obj);
    }
    
    // Process all children
    for (auto& child : children) {
        child->collectVisibleObjects(visibleObjects, frustum);
    }
}

// ----- OctreeNode Implementation -----

OctreeNode::OctreeNode(const AABB& bounds, int maxDepth, int maxObjects, int currentDepth, OctreeNode* parent)
    : bounds(bounds),
      maxDepth(maxDepth),
      currentDepth(currentDepth),
      maxObjectsPerNode(maxObjects),
      isLeaf(true),
      parent(parent) {
}

void OctreeNode::split() {
    if (!isLeaf) return;
    
    isLeaf = false;
    
    glm::vec3 center = bounds.getCenter();
    glm::vec3 extents = bounds.getExtents();
    
    // Create 8 children for the octree
    for (int i = 0; i < 8; ++i) {
        glm::vec3 childMin, childMax;
        
        childMin.x = (i & 1) ? center.x : bounds.min.x;
        childMin.y = (i & 2) ? center.y : bounds.min.y;
        childMin.z = (i & 4) ? center.z : bounds.min.z;
        
        childMax.x = (i & 1) ? bounds.max.x : center.x;
        childMax.y = (i & 2) ? bounds.max.y : center.y;
        childMax.z = (i & 4) ? bounds.max.z : center.z;
        
        AABB childBounds(childMin, childMax);
        children[i] = std::make_unique<OctreeNode>(childBounds, maxDepth, maxObjectsPerNode, currentDepth + 1, this);
    }
    
    // Redistribute objects to children
    for (auto* obj : objects) {
        // Extract position from model matrix instead of using getPosition()
        glm::mat4 objMatrix = obj->getModelMatrix();
        glm::vec3 pos(objMatrix[3][0], objMatrix[3][1], objMatrix[3][2]);
        
        int octant = getOctantForPoint(pos);
        if (octant >= 0) {
            children[octant]->insert(obj);
        }
    }
    
    // Clear objects from this node since they're now in children
    // We only keep objects that don't fit in a single child
    std::vector<GameObject*> remainingObjects;
    for (auto* obj : objects) {
        // Extract position from model matrix
        glm::mat4 objMatrix = obj->getModelMatrix();
        glm::vec3 pos(objMatrix[3][0], objMatrix[3][1], objMatrix[3][2]);
        
        int octant = getOctantForPoint(pos);
        if (octant < 0) {
            remainingObjects.push_back(obj);
        }
    }
    
    objects = std::move(remainingObjects);
}

int OctreeNode::getOctantForPoint(const glm::vec3& point) const {
    glm::vec3 center = bounds.getCenter();
    
    // Check if point is on a boundary
    if (point.x == center.x || point.y == center.y || point.z == center.z) {
        return -1;
    }
    
    int octant = 0;
    if (point.x > center.x) octant |= 1;
    if (point.y > center.y) octant |= 2;
    if (point.z > center.z) octant |= 4;
    
    return octant;
}

void OctreeNode::insert(GameObject* obj) {
    // Extract position from model matrix
    glm::mat4 objMatrix = obj->getModelMatrix();
    glm::vec3 pos(objMatrix[3][0], objMatrix[3][1], objMatrix[3][2]);
    
    // Check if this object fits in this node
    if (!bounds.contains(pos)) {
        return;
    }
    
    // If we're a leaf and below capacity, add the object here
    if (isLeaf) {
        objects.push_back(obj);
        
        // Check if we should split
        if (currentDepth < maxDepth && objects.size() > maxObjectsPerNode) {
            split();
        }
        return;
    }
    
    // Otherwise, try to add to a child
    int octant = getOctantForPoint(pos);
    if (octant >= 0) {
        children[octant]->insert(obj);
    } else {
        // Object is on a boundary, keep it in this node
        objects.push_back(obj);
    }
}

void OctreeNode::remove(GameObject* obj) {
    // Extract position from model matrix
    glm::mat4 objMatrix = obj->getModelMatrix();
    glm::vec3 pos(objMatrix[3][0], objMatrix[3][1], objMatrix[3][2]);
    
    // Check if this object could be in this node
    if (!bounds.contains(pos)) {
        return;
    }
    
    // Try to remove from this node's objects
    auto it = std::find(objects.begin(), objects.end(), obj);
    if (it != objects.end()) {
        objects.erase(it);
        return;
    }
    
    // If not found and not a leaf, try to remove from children
    if (!isLeaf) {
        int octant = getOctantForPoint(pos);
        if (octant >= 0) {
            children[octant]->remove(obj);
        } else {
            // Object is on a boundary, try all children
            for (int i = 0; i < 8; ++i) {
                children[i]->remove(obj);
            }
        }
    }
}

void OctreeNode::update(GameObject* obj) {
    // Extract position from model matrix
    glm::mat4 objMatrix = obj->getModelMatrix();
    glm::vec3 pos(objMatrix[3][0], objMatrix[3][1], objMatrix[3][2]);
    
    // Check if the object is still in this node
    if (bounds.contains(pos)) {
        // Remove and reinsert
        remove(obj);
        insert(obj);
    } else if (parent) {
        // Object has moved outside this node, let parent handle it
        parent->update(obj);
    }
}

void OctreeNode::collectVisibleObjects(std::vector<GameObject*>& visibleObjects, const Frustum& frustum) {
    // Early out if this node is completely outside the frustum
    if (!frustum.containsAABB(bounds)) {
        return;
    }
    
    // Add all objects in this node
    for (auto* obj : objects) {
        visibleObjects.push_back(obj);
    }
    
    // Recursively check children if not a leaf
    if (!isLeaf) {
        for (int i = 0; i < 8; ++i) {
            children[i]->collectVisibleObjects(visibleObjects, frustum);
        }
    }
}

void OctreeNode::clear() {
    objects.clear();
    
    if (!isLeaf) {
        for (int i = 0; i < 8; ++i) {
            children[i]->clear();
        }
    }
}

const AABB& OctreeNode::getBounds() const {
    return bounds;
}

// ----- SceneGraph Implementation -----

SceneGraph::SceneGraph(const AABB& worldBounds)
    : worldBounds(worldBounds) {
    rootNode = std::make_unique<SceneNode>();
    octreeRoot = std::make_unique<OctreeNode>(worldBounds);
}

void SceneGraph::addObject(GameObject* obj, SceneNode* parent) {
    // Add to scene hierarchy
    SceneNode* targetNode = parent ? parent : rootNode.get();
    targetNode->attachObject(obj);
    
    // Add to spatial structure
    octreeRoot->insert(obj);
}

void SceneGraph::removeObject(GameObject* obj) {
    // Define the function variable first to allow self-reference
    std::function<bool(SceneNode*)> removeFromHierarchy;
    
    // Then define the function with a reference to itself
    removeFromHierarchy = [obj, &removeFromHierarchy](SceneNode* node) -> bool {
        // Check this node's objects
        auto it = std::find(node->getObjects().begin(), node->getObjects().end(), obj);
        if (it != node->getObjects().end()) {
            node->detachObject(obj);
            return true;
        }
        
        // Check children
        for (auto& child : node->children) {
            if (removeFromHierarchy(child.get())) {
                return true;
            }
        }
        
        return false;
    };
    
    // Call the function
    removeFromHierarchy(rootNode.get());
    
    // Remove from spatial structure
    octreeRoot->remove(obj);
}

void SceneGraph::updateTransforms() {
    rootNode->updateWorldTransform();
}

void SceneGraph::render(Renderer& renderer, const Camera& camera) {
    // Create frustum from camera
    Frustum frustum;
    frustum.updateFromCamera(camera);
    
    // Collect visible objects
    std::vector<GameObject*> visibleObjects;
    getVisibleObjects(visibleObjects, camera);
    
    // Submit them to the renderer
    for (auto* obj : visibleObjects) {
        renderer.submit(obj);
    }
}

SceneNode* SceneGraph::createNode(SceneNode* parent) {
    auto node = std::make_unique<SceneNode>();
    SceneNode* nodePtr = node.get();
    
    if (parent) {
        parent->addChild(std::move(node));
    } else {
        rootNode->addChild(std::move(node));
    }
    
    return nodePtr;
}

void SceneGraph::destroyNode(SceneNode* node) {
    if (!node || node == rootNode.get()) {
        return;
    }
    
    SceneNode* parent = node->getParent();
    if (parent) {
        parent->removeChild(node);
    }
}

SceneNode* SceneGraph::getRootNode() const {
    return rootNode.get();
}

void SceneGraph::getVisibleObjects(std::vector<GameObject*>& visibleObjects, const Camera& camera) {
    Frustum frustum;
    frustum.updateFromCamera(camera);
    
    // Use octree for efficient frustum culling
    octreeRoot->collectVisibleObjects(visibleObjects, frustum);
}

void SceneGraph::updateSpatialStructure() {
    // Clear and rebuild octree
    octreeRoot = std::make_unique<OctreeNode>(worldBounds);
    
    // Helper function to recursively add all objects from the scene hierarchy
    std::function<void(SceneNode*)> addNodeObjects = [&](SceneNode* node) {
        for (auto* obj : node->getObjects()) {
            octreeRoot->insert(obj);
        }
        
        // Process children
        for (const auto& child : node->children) {
            addNodeObjects(child.get());
        }
    };
    
    addNodeObjects(rootNode.get());
}