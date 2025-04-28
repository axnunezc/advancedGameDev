#ifndef DEFERRED_RENDERER_HPP
#define DEFERRED_RENDERER_HPP

#include "Framebuffer.hpp"
#include "Shader.hpp"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class DeferredRenderer {
private:
    Framebuffer gBuffer;       // G-Buffer for deferred rendering
    GLuint quadVAO;            // VAO for screen-space quad
    Shader geometryShader;     // Shader for geometry pass
    Shader lightingShader;     // Shader for lighting pass - renamed from 'lightingPass'
    int screenWidth;           // Screen width
    int screenHeight;          // Screen height
    
    // Helper function to create a screen quad
    GLuint createScreenQuad();
    
public:
    // Constructor: Create deferred rendering system with shaders
    DeferredRenderer(int width, int height, 
                     const std::string& geoVertPath, const std::string& geoFragPath,
                     const std::string& lightVertPath, const std::string& lightFragPath);
    
    // Destructor
    ~DeferredRenderer();
    
    // Begin the geometry pass
    void geometryPassBegin();
    
    // End the geometry pass
    void geometryPassEnd();
    
    // Perform the lighting pass (renamed from 'lightingPass')
    void renderLighting(const glm::vec3& lightPos, const glm::vec3& lightColor, const glm::vec3& viewPos);
    
    // Set transformation matrices for geometry shader
    void setMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj);
    
    // Set bone matrices for skeletal animation
    void setBoneMatrices(const std::vector<glm::mat4>& boneMatrices, bool hasArmature);
    
    // Render a specific G-Buffer texture to the screen
    void renderGBufferTexture(int textureIndex);
    
    // Get the geometry pass shader
    Shader& getGeometryShader();
    
    // Get the lighting pass shader
    Shader& getLightingShader();
};

#endif // DEFERRED_RENDERER_HPP