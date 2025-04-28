#include "QuadRenderer.hpp"
#include <iostream>

QuadRenderer::QuadRenderer() {
    // Create a quad mesh with two triangles
    // The vertices are specified in NDC (Normalized Device Coordinates)
    float quadVertices[] = {
        // First triangle (positions)
        -1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        
        // Second triangle (positions)
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f
    };
    
    // Create VAO and VBO
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    // Bind VAO
    glBindVertexArray(vao);
    
    // Bind VBO and upload data
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Set vertex attribute pointers
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    std::cout << "QuadRenderer initialized successfully" << std::endl;
}

QuadRenderer::~QuadRenderer() {
    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

void QuadRenderer::renderQuad() {
    // Disable depth testing for fullscreen quad
    glDisable(GL_DEPTH_TEST);
    
    // Bind VAO and draw
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
    
    // Unbind
    glBindVertexArray(0);
}