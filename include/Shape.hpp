#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <GL/glew.h>
#include <vector>
#include <string>

// Bone structure to store information about each bone
struct Bone {
    std::string name;
    int parentIndex;
    glm::vec3 localPosition;       // Original bone head position
    glm::vec3 parentToChildVector; // Vector from parent bone to this bone
};

// Structure to store bone weights and indices for each vertex
struct VertexBoneData {
    int indices[4] = {0, 0, 0, 0};
    float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

class Shape {
private:
    GLuint vao;
    GLuint vbo;
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> norm;
    std::vector<glm::vec2> uv;  // Added UV support
    
    // Bone related data
    bool hasBones = false;
    std::vector<Bone> bones;
    std::vector<VertexBoneData> vertexBoneData;
    std::vector<glm::mat4> boneMatrices;  // Current bone transformation matrices

public:
    // Constructors and destructor
    Shape(const size_t triangleCount = 0, const std::vector<float>& vertexData = std::vector<float>());
    
    // New constructor for mesh with armature data
    Shape(const size_t vertexCount, 
          const std::vector<float>& positionData,
          const std::vector<float>& normalData,
          const std::vector<float>& uvData,
          const std::vector<Bone>& bones,
          const std::vector<VertexBoneData>& vertexBoneData,
          bool hasBones);
          
    ~Shape();
    
    // Get vertex data accessors
    const std::vector<glm::vec3>& getPositions() const { return pos; }
    const std::vector<glm::vec3>& getNormals() const { return norm; }
    const std::vector<glm::vec2>& getUVs() const { return uv; }
    bool hasVertexData() const { return !pos.empty(); }
    size_t getVertexCount() const { return pos.size(); }
    
    // Bone related accessors
    bool hasArmature() const { return hasBones; }
    const std::vector<Bone>& getBones() const { return bones; }
    size_t getBoneCount() const { return bones.size(); }
    const std::vector<VertexBoneData>& getVertexBoneData() const { return vertexBoneData; }
    
    // Update bone transformations for animation
    void updateBoneTransforms(const std::vector<glm::quat>& boneRotations);
    
    // Get bone matrices for shader
    const std::vector<glm::mat4>& getBoneMatrices() const { return boneMatrices; }
    
    // OpenGL buffer accessors
    GLuint getVAO() const { return vao; }
    GLuint getVBO() const { return vbo; }
};

// Load mesh data from file (old format)
bool loadMeshData(const std::string& filename, size_t& triangleCount, std::vector<float>& vertexData);

// Load mesh data from file (new format with armature support)
bool loadMeshWithArmature(const std::string& filename, 
                         size_t& vertexCount, 
                         size_t& faceCount, 
                         std::vector<float>& positionData,
                         std::vector<float>& normalData, 
                         std::vector<float>& uvData,
                         std::vector<Bone>& bones,
                         std::vector<VertexBoneData>& vertexBoneData,
                         bool& hasBones);

// Helper function to create a Shape from file
Shape* createShapeFromFile(const std::string& filename);

#endif // SHAPE_HPP