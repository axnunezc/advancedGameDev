#include "GameObject.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include "TypeRegistry.hpp"

// Implementation of getGameObjectTypeId
int getGameObjectTypeId(const GameObject& obj) {
    return TypeRegistry::getInstance()->registerType(typeid(obj).name());
}

GameObject::GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id)
    : position(pos),
      rotation(rot),
      scale(1.0f, 1.0f, 1.0f),        // Initialize scale to (1,1,1)
      renderElementShape(shape),
      renderElement(id),
      updateFunction(nullptr),
      velocity(0.0f),                 // Initialize physics properties
      angularVelocity(0.0f),
      mass(1.0f),
      inverseMass(1.0f),
      isStatic(false),
      boundsDirty(true)               // Start with dirty bounds to force initial calculation
{
    // Initialize model matrix using our helper method
    updateModelMatrix();
                  
    // Initialize bounding box (will be updated when first accessed)
    boundingBox = AABB(position - glm::vec3(0.5f), position + glm::vec3(0.5f));
}

void GameObject::updateModelMatrix() {
  // Construct the model matrix including scale
  glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
  glm::mat4 rotationMatrix = rotation.toMatrix();
  glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
  
  // Combine the transformations: translate * rotate * scale
  modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
  
  // Update bone transforms if we have an armature
  if (hasArmature) {
      updateBoneTransforms();
  }
  
  // Mark bounds as dirty
  boundsDirty = true;
}

void GameObject::update(float deltaTime) {
    // Use custom update function if available
    if (updateFunction) {
        updateFunction(this, deltaTime);
        return;
    }
    
    // No custom update - do nothing by default
    // Physics integration happens in PhysicsIntegrator::updateObject
}

const AABB& GameObject::getBoundingBox() {
    if (boundsDirty) {
        updateBoundingBox();
    }
    return boundingBox;
}

void GameObject::updateBoundingBox() {
    // If the shape has vertex data
    if (renderElementShape.hasVertexData()) {
        const auto& positions = renderElementShape.getPositions();
        
        // Start with first vertex transformed to world space
        glm::vec4 firstVertex = modelMatrix * glm::vec4(positions[0], 1.0f);
        glm::vec3 worldPos = glm::vec3(firstVertex) / firstVertex.w;
        
        glm::vec3 min = worldPos;
        glm::vec3 max = worldPos;
        
        // Process remaining vertices
        for (size_t i = 1; i < positions.size(); ++i) {
            glm::vec4 transformedVertex = modelMatrix * glm::vec4(positions[i], 1.0f);
            worldPos = glm::vec3(transformedVertex) / transformedVertex.w;
            
            // Expand bounding box
            min = glm::min(min, worldPos);
            max = glm::max(max, worldPos);
        }
        
        // Add a small margin to improve culling robustness
        const float margin = 0.05f;
        min -= glm::vec3(margin);
        max += glm::vec3(margin);
        
        // Update the bounding box
        boundingBox = AABB(min, max);
    }
    else {
        // Fallback if no shape data: create bounding box around position
        float radius = 0.5f * glm::max(glm::max(scale.x, scale.y), scale.z);  // Account for scale
        boundingBox = AABB(position - glm::vec3(radius), position + glm::vec3(radius));
    }
    
    boundsDirty = false;
}

void GameObject::initBoneData() {
  // Reset bone data
  boneTransforms.clear();
  boneMatrices.clear();
  hasArmature = false;
  
  // Check if shape has bone data using the public accessor
  if (!renderElementShape.hasArmature()) {
      return;
  }
  
  // Use the public accessor to get the bones
  const auto& bones = renderElementShape.getBones();
  if (bones.empty()) {
      return;
  }
  
  // We have bone data
  hasArmature = true;
  
  // Resize containers
  boneTransforms.resize(bones.size());
  boneMatrices.resize(bones.size(), glm::mat4(1.0f));
  
  // Initialize bone transforms
  for (size_t i = 0; i < bones.size(); i++) {
      const Bone& bone = bones[i];
      
      BoneTransform& transform = boneTransforms[i];
      transform.initialRotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
      transform.currentRotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
      transform.localPosition = bone.localPosition;
      transform.parentIndex = bone.parentIndex;
      
      // Create offset matrix (translation to bone position)
      transform.offsetMatrix = glm::translate(glm::mat4(1.0f), bone.localPosition);
      transform.finalTransform = transform.offsetMatrix; // Initialize with offset
  }
  
  // Initial update of bone transforms
  updateBoneTransforms();
}

void GameObject::updateBoneRotations(const std::map<int, Quaternion>& rotations) {
  if (!hasArmature) return;
  
  // Apply rotations to bones
  for (const auto& pair : rotations) {
      int boneId = pair.first;
      
      if (boneId >= 0 && boneId < static_cast<int>(boneTransforms.size())) {
          boneTransforms[boneId].currentRotation = pair.second;
      }
  }
  
  // Update the transforms
  updateBoneTransforms();
  
  // Mark bounding box as dirty since bones changed
  boundsDirty = true;
}

void GameObject::updateBoneTransforms() {
  if (!hasArmature) return;
  
  // Recursively compute bone transforms in hierarchy order
  for (size_t i = 0; i < boneTransforms.size(); i++) {
      BoneTransform& transform = boneTransforms[i];
      
      // Create local transform from position and rotation
      glm::mat4 localRotation = transform.currentRotation.toMatrix();
      glm::mat4 localTransform = transform.offsetMatrix * localRotation;
      
      // Apply parent transform if this bone has a parent
      if (transform.parentIndex >= 0 && 
          transform.parentIndex < static_cast<int>(boneTransforms.size())) {
          // Combine with parent transform
          transform.finalTransform = 
              boneTransforms[transform.parentIndex].finalTransform * localTransform;
      } else {
          // Root bone - combine with object's transform
          transform.finalTransform = modelMatrix * localTransform;
      }
      
      // Store the final transform matrix for this bone
      boneMatrices[i] = transform.finalTransform;
  }
}