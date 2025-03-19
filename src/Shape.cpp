#include "Shape.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

Shape::Shape(const size_t triangleCount, const std::vector<float>& vertexData) {
    if (triangleCount == 0 || vertexData.empty()) {
        std::cerr << "Error: Empty mesh data!" << std::endl;
        return;
    }

    size_t totalVertices = triangleCount * 3;

    if (vertexData.size() != totalVertices * 6) {
        std::cerr << "Error: Incorrect vertex data size!" << std::endl;
        return;
    }

    // Extract position and normal data into pos and norm vectors
    for (size_t i = 0; i < totalVertices; i++) {
        float x = vertexData[i * 6 + 0];
        float y = vertexData[i * 6 + 1];
        float z = vertexData[i * 6 + 2];
        pos.emplace_back(x, y, z);

        float nx = vertexData[i * 6 + 3];
        float ny = vertexData[i * 6 + 4];
        float nz = vertexData[i * 6 + 5];
        norm.emplace_back(nx, ny, nz);
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1); // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(totalVertices * 3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "Shape successfully created with " << triangleCount << " triangles." << std::endl;
}

Shape::~Shape() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    vao = 0;
    vbo = 0;
    std::cout << "Shape destroyed, OpenGL buffers deleted." << std::endl;
}

bool loadMeshData(const std::string& filename, size_t& triangleCount, std::vector<float>& vertexData) {
    std::ifstream input(filename);
    if (!input.is_open()) {
        std::cerr << "Error: Could not open mesh file " << filename << std::endl;
        return false;
    }

    std::string line;
    
    // Read number of triangles
    if (!std::getline(input, line)) {
        std::cerr << "Error: File format incorrect!" << std::endl;
        return false;
    }
    triangleCount = std::stoi(line);

    // Read vertex positions first
    for (size_t i = 0; i < triangleCount * 3; ++i) {
        float x, y, z;
        if (!(input >> x >> y >> z)) {
            std::cerr << "Error: Invalid vertex position data!" << std::endl;
            return false;
        }
        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(z);
    }

    // Read normal vectors next
    for (size_t i = 0; i < triangleCount * 3; ++i) {
        float nx, ny, nz;
        if (!(input >> nx >> ny >> nz)) {
            std::cerr << "Error: Invalid normal data!" << std::endl;
            return false;
        }
        vertexData.push_back(nx);
        vertexData.push_back(ny);
        vertexData.push_back(nz);
    }

    input.close();
    std::cout << "Loaded " << triangleCount << " triangles from " << filename << std::endl;
    return true;
}