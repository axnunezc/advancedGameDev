#ifndef SHAPE_HPP
#define SHAPE_HPP

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Shape {
private:
    GLuint vao, vbo;
    size_t vertexCount;
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> norm;

public:
    Shape(const size_t triangleCount, const std::vector<float>& vertexData);
    ~Shape();

    void bind() const;
    void unbind() const;
    size_t getVertexCount() const;

    GLuint getVAO() const {
        return vao;
    }

    GLuint getVBO() const {
        return vbo;
    }

    static bool loadMeshData(const std::string& filename, size_t& triangleCount, std::vector<float>& vertexData);
};

#endif
