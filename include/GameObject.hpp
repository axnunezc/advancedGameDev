#ifndef GAMEOBJECT_HPP
#define GAMEOBJECT_HPP
#include "Quaternion.hpp"
#include "Shape.hpp"
#include "AABB.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>
#include <typeinfo>
#include <string>
#include <map>

// Macro for derived classes to declare their type ID
#define DECLARE_GAMEOBJECT_TYPE() \
    int getTypeId() const override { return getGameObjectTypeId(*this); }

class GameObject {
public:
    GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id);
    
    // Physical properties
    glm::vec3 position;
    Quaternion rotation;
    glm::vec3 scale;       // Added scale property
    glm::vec3 velocity;
    glm::vec3 angularVelocity;
    float mass;
    float inverseMass; // Precomputed for efficiency
    bool isStatic;     // If true, object doesn't move
    
    // Rendering properties
    Shape renderElementShape;
    int renderElement;  // This could be an ID or an object reference
    glm::mat4 modelMatrix;
    
    // Update logic
    using UpdateFunction = std::function<void(GameObject*, float)>;
    UpdateFunction updateFunction;
    
    // Methods for accessing properties
    glm::vec3 getPosition() const { return position; }
    void setPosition(const glm::vec3& pos) { 
        position = pos; 
        boundsDirty = true; 
        updateModelMatrix();
    }
    
    Quaternion& getRotation() { return rotation; }
    void setRotation(const Quaternion& rot) { 
        rotation = rot; 
        boundsDirty = true; 
        updateModelMatrix();
    }
    
    glm::vec3 getScale() const { return scale; }
    void setScale(const glm::vec3& newScale) {
        scale = newScale;
        boundsDirty = true;
        updateModelMatrix();
    }
    
    glm::vec3 getVelocity() const { return velocity; }
    void setVelocity(const glm::vec3& vel) { velocity = vel; }
    
    glm::vec3 getAngularVelocity() const { return angularVelocity; }
    void setAngularVelocity(const glm::vec3& angVel) { angularVelocity = angVel; }
    
    float getMass() const { return mass; }
    void setMass(float m) { 
        mass = m; 
        if (m > 0.0001f) {
            inverseMass = 1.0f / m;
            isStatic = false;
        } else {
            inverseMass = 0.0f;
            isStatic = true;
        }
    }
    
    bool getIsStatic() const { return isStatic; }
    void setIsStatic(bool static_) { 
        isStatic = static_; 
        if (static_) {
            inverseMass = 0.0f;
        } else if (mass > 0.0001f) {
            inverseMass = 1.0f / mass;
        }
    }
    
    // Rendering methods
    GLuint getVAO() const { return renderElementShape.getVAO(); }
    GLuint getVBO() const { return renderElementShape.getVBO(); }
    int getVertexCount() const { return renderElementShape.getVertexCount(); }
    const glm::mat4& getModelMatrix() const { return modelMatrix; }
    
    // Update method (called once per frame)
    void update(float deltaTime);
    
    // Set custom update behavior
    void setUpdateFunction(UpdateFunction func) { updateFunction = func; }
    
    // Bounding volume functionality
    AABB boundingBox;
    bool boundsDirty;
    
    // Methods to manage bounding box
    void markBoundsDirty() { boundsDirty = true; }
    const AABB& getBoundingBox();
    void updateBoundingBox();
    
    // Type identification for collision detection
    virtual int getTypeId() const { return -1; }

    Shape& getShape() {
        return renderElementShape;
    }

    // Animation-related methods
    void initBoneData();
    void updateBoneRotations(const std::map<int, Quaternion>& rotations);
    void updateBoneTransforms();
    const std::vector<glm::mat4>& getBoneMatrices() const { return boneMatrices; }
    bool hasAnimatableSkeleton() const { return hasArmature && !boneTransforms.empty(); }

    // Bone data structure
    struct BoneTransform {
        Quaternion initialRotation;   // Initial bind pose rotation
        Quaternion currentRotation;   // Current animation rotation
        glm::vec3 localPosition;      // Position in local space
        int parentIndex;              // Index of parent bone (-1 for root)
        glm::mat4 offsetMatrix;       // Transform from bind pose
        glm::mat4 finalTransform;     // Final world space transform
    };

protected:
    // Animation-related members
    std::vector<BoneTransform> boneTransforms;
    std::vector<glm::mat4> boneMatrices;  // Final matrices sent to shader
    bool hasArmature;
    
private:
    // Helper to update the model matrix when position, rotation, or scale changes
    void updateModelMatrix();
};

// Helper function for type identification
int getGameObjectTypeId(const GameObject& obj);

#endif // GAMEOBJECT_HPP