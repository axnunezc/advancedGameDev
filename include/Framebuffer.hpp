#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include <GL/glew.h>
#include <vector>
#include <stdexcept>
#include <iostream>

// Structure to define texture properties for FBO attachments
struct TextureProperties {
    // Format of the internal texture (e.g., GL_RGBA, GL_RGB16F, etc.)
    GLenum internalFormat = GL_RGBA;
    
    // Format and type of the pixel data (used when creating the texture)
    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;
    
    // Texture filtering parameters
    GLenum minFilter = GL_LINEAR;
    GLenum magFilter = GL_LINEAR;
    
    // Texture wrapping parameters
    GLenum wrapS = GL_CLAMP_TO_EDGE;
    GLenum wrapT = GL_CLAMP_TO_EDGE;
    
    // Constructor with commonly used parameters
    TextureProperties(
        GLenum internalFormat = GL_RGBA,
        GLenum format = GL_RGBA,
        GLenum type = GL_UNSIGNED_BYTE,
        GLenum minFilter = GL_LINEAR,
        GLenum magFilter = GL_LINEAR
    ) : internalFormat(internalFormat), 
        format(format), 
        type(type), 
        minFilter(minFilter), 
        magFilter(magFilter) {}
};

class Framebuffer {
private:
    GLuint fbo;                  // Framebuffer object ID
    std::vector<GLuint> tex;     // Texture attachments
    GLuint rbo;                  // Renderbuffer object for depth/stencil
    int resX;                    // Width of the framebuffer
    int resY;                    // Height of the framebuffer
    bool hasDepthStencil;        // Whether this FBO has a depth/stencil attachment

public:
    // Constructor - creates FBO with specified resolution and texture properties
    Framebuffer(int width, int height, const std::vector<TextureProperties>& textureProps = {}, bool createDepthStencil = true);
    
    // Destructor - cleans up OpenGL resources
    ~Framebuffer();
    
    // Get the FBO ID
    GLuint getFBO() const;
    
    // Bind the FBO for rendering
    void bindFBO() const;
    
    // Bind all textures starting from specified texture unit
    void bindTextures(GLuint startUnit = 0) const;
    
    // Get specific texture ID
    GLuint getTexture(size_t index) const;
    
    // Get number of texture attachments
    size_t getTextureCount() const;
    
    // Get resolution
    int getWidth() const { return resX; }
    int getHeight() const { return resY; }
    
    // Utility function to check FBO status
    static bool checkFBOStatus();
};

#endif // FRAMEBUFFER_HPP