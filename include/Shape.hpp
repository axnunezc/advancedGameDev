#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

class Shape {
private:
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> norm;
    
    unsigned int vao;
    unsigned int vbo;
    
public:
    // Constructor
    Shape(const size_t triangleCount, const std::vector<float>& vertexData);
    
    // Destructor
    ~Shape();
    
    // Methods
    unsigned int getVAO() const { return vao; }
    unsigned int getVBO() const { return vbo; }
    int getVertexCount() const { return pos.size(); }
    
    // Added getter for vertices - needed for AABB calculation
    const std::vector<glm::vec3>& getVertices() const { return pos; }
    const std::vector<glm::vec3>& getNormals() const { return norm; }
};

// Function to load mesh data from file
bool loadMeshData(const std::string& filename, size_t& triangleCount, std::vector<float>& vertexData);

// Helper functions to create common shapes
Shape createQuadShape();
Shape createCircleShape(int segments);

#endif // SHAPE_HPP