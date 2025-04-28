#include "Framebuffer.hpp"

Framebuffer::Framebuffer(int width, int height, const std::vector<TextureProperties>& textureProps, bool createDepthStencil) 
    : resX(width), resY(height), hasDepthStencil(createDepthStencil), rbo(0) {
    
    // Check for maximum number of color attachments
    const size_t MAX_COLOR_ATTACHMENTS = 8;
    if (textureProps.size() > MAX_COLOR_ATTACHMENTS) {
        std::cerr << "Warning: Attempting to create FBO with " << textureProps.size() 
                  << " textures, but maximum portable number is " << MAX_COLOR_ATTACHMENTS << std::endl;
    }
    
    // Generate framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    // Create and attach textures
    tex.resize(textureProps.size());
    glGenTextures(static_cast<GLsizei>(tex.size()), tex.data());
    
    // Keep track of which attachments we're using
    std::vector<GLenum> drawBuffers;
    
    // Create and attach each texture
    for (size_t i = 0; i < tex.size(); ++i) {
        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, tex[i]);
        
        // Set texture properties
        const TextureProperties& props = textureProps[i];
        
        // Create texture storage (no data)
        glTexImage2D(GL_TEXTURE_2D, 0, props.internalFormat, resX, resY, 0, props.format, props.type, nullptr);
        
        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, props.minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, props.magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, props.wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, props.wrapT);
        
        // Attach texture to framebuffer
        GLenum attachment = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex[i], 0);
        
        // Add to draw buffers list
        drawBuffers.push_back(attachment);
    }
    
    // Set draw buffers if we have any color attachments
    if (!drawBuffers.empty()) {
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    }
    
    // Create renderbuffer for depth and stencil if requested
    if (createDepthStencil) {
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resX, resY);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    }
    
    // Check if framebuffer is complete
    if (!checkFBOStatus()) {
        throw std::runtime_error("Framebuffer creation failed!");
    }
    
    // Unbind the framebuffer to return to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() {
    // Delete texture attachments
    if (!tex.empty()) {
        glDeleteTextures(static_cast<GLsizei>(tex.size()), tex.data());
    }
    
    // Delete renderbuffer if it exists
    if (rbo) {
        glDeleteRenderbuffers(1, &rbo);
    }
    
    // Delete framebuffer
    glDeleteFramebuffers(1, &fbo);
    
    // Clear data
    tex.clear();
    rbo = 0;
    fbo = 0;
}

GLuint Framebuffer::getFBO() const {
    return fbo;
}

void Framebuffer::bindFBO() const {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, resX, resY);
}

void Framebuffer::bindTextures(GLuint startUnit) const {
    for (size_t i = 0; i < tex.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + startUnit + static_cast<GLuint>(i));
        glBindTexture(GL_TEXTURE_2D, tex[i]);
    }
}

GLuint Framebuffer::getTexture(size_t index) const {
    if (index >= tex.size()) {
        throw std::out_of_range("Texture index out of range");
    }
    return tex[index];
}

size_t Framebuffer::getTextureCount() const {
    return tex.size();
}

bool Framebuffer::checkFBOStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer error: ";
        
        switch (status) {
            case GL_FRAMEBUFFER_UNDEFINED:
                std::cerr << "GL_FRAMEBUFFER_UNDEFINED";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                std::cerr << "GL_FRAMEBUFFER_UNSUPPORTED";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                std::cerr << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
                break;
            default:
                std::cerr << "Unknown framebuffer status error";
        }
        std::cerr << std::endl;
        return false;
    }
    return true;
}