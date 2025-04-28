#ifndef QUAD_RENDERER_HPP
#define QUAD_RENDERER_HPP

#include <GL/glew.h>

class QuadRenderer {
private:
    GLuint vao;        // Vertex Array Object
    GLuint vbo;        // Vertex Buffer Object

public:
    // Constructor
    QuadRenderer();
    
    // Destructor
    ~QuadRenderer();
    
    // Render a fullscreen quad
    void renderQuad();
};

#endif // QUAD_RENDERER_HPP