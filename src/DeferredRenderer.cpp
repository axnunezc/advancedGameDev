#include "DeferredRenderer.hpp"
#include <iostream>

DeferredRenderer::DeferredRenderer(int width, int height, 
                                 const std::string& geoVertPath, const std::string& geoFragPath,
                                 const std::string& lightVertPath, const std::string& lightFragPath) 
    : screenWidth(width), screenHeight(height),
      geometryShader(geoVertPath, geoFragPath),
      lightingShader(lightVertPath, lightFragPath),
      // Initialize gBuffer in the initialization list
      gBuffer(width, height, {
            TextureProperties(GL_RGB32F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST),  // Diffuse
            TextureProperties(GL_RGB32F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST),  // Normal
            TextureProperties(GL_RGB32F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST)   // Position
      }, true)
{
    // Create screen quad for lighting pass
    quadVAO = createScreenQuad();
    
    // Set up lighting shader uniforms (using uniform instead of setInt)
    lightingShader.use();
    glUniform1i(glGetUniformLocation(lightingShader.program, "gDiffuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(lightingShader.program, "gPosition"), 2);
    
    std::cout << "G-Buffer created successfully with 3 attachments" << std::endl;
}

DeferredRenderer::~DeferredRenderer() {
    glDeleteVertexArrays(1, &quadVAO);
}

GLuint DeferredRenderer::createScreenQuad() {
    // Vertex data for a screen-filling quad (-1 to 1 in NDC)
    float quadVertices[] = {
        // positions        // texture coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };
    
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    
    glBindVertexArray(quadVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    return quadVAO;
}

void DeferredRenderer::geometryPassBegin() {
    // Bind G-Buffer for writing
    gBuffer.bindFBO();
    
    // Clear G-Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use geometry shader
    geometryShader.use();
}

void DeferredRenderer::geometryPassEnd() {
    // Nothing specific needed here, just a logical separation
}

// Renamed from lightingPass to renderLighting
void DeferredRenderer::renderLighting(const glm::vec3& lightPos, const glm::vec3& lightColor, const glm::vec3& viewPos) {
    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Clear default framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use lighting shader
    lightingShader.use();
    
    // Bind G-Buffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(0)); // Diffuse
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(1)); // Normal
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(2)); // Position
    
    // Set light properties using glUniform instead of setVec3
    glUniform3fv(glGetUniformLocation(lightingShader.program, "lightPos"), 1, &lightPos[0]);
    glUniform3fv(glGetUniformLocation(lightingShader.program, "lightColor"), 1, &lightColor[0]);
    glUniform3fv(glGetUniformLocation(lightingShader.program, "viewPos"), 1, &viewPos[0]);
    
    // Disable depth test for screen-space quad
    glDisable(GL_DEPTH_TEST);
    
    // Render quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
}

void DeferredRenderer::setBoneMatrices(const std::vector<glm::mat4>& boneMatrices, bool hasArmature) {
    geometryShader.use();
    
    // Set hasArmature flag using glUniform
    glUniform1i(glGetUniformLocation(geometryShader.program, "hasArmature"), hasArmature ? 1 : 0);
    
    if (hasArmature) {
        // Set bone count
        int boneCount = static_cast<int>(boneMatrices.size());
        glUniform1i(glGetUniformLocation(geometryShader.program, "boneCount"), boneCount);
        
        // Set bone matrices
        for (int i = 0; i < boneCount; i++) {
            std::string uniformName = "boneMatrices[" + std::to_string(i) + "]";
            glUniformMatrix4fv(
                glGetUniformLocation(geometryShader.program, uniformName.c_str()),
                1, GL_FALSE, &boneMatrices[i][0][0]
            );
        }
    }
}

void DeferredRenderer::renderGBufferTexture(int textureIndex) {
    // Make sure the index is valid
    if (textureIndex < 0 || textureIndex >= 3) {
        std::cerr << "Invalid G-Buffer texture index: " << textureIndex << std::endl;
        return;
    }
    
    // Bind default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    
    // Clear default framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use a simple shader that just outputs the texture
    static const char* vertexShaderSource = R"(
    #version 410
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    void main() {
        gl_Position = vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
    )";
    
    static const char* fragmentShaderSource = R"(
    #version 410
    out vec4 FragColor;
    in vec2 TexCoord;
    uniform sampler2D gTexture;
    void main() {
        vec3 texColor = texture(gTexture, TexCoord).rgb;
        FragColor = vec4(texColor, 1.0);
    }
    )";
    
    // Create shader if needed
    static GLuint shaderProgram = 0;
    if (shaderProgram == 0) {
        // Create shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    
    // Use shader
    glUseProgram(shaderProgram);
    
    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(textureIndex));
    glUniform1i(glGetUniformLocation(shaderProgram, "gTexture"), 0);
    
    // Disable depth test for screen-space quad
    glDisable(GL_DEPTH_TEST);
    
    // Render quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);
}

void DeferredRenderer::setMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& proj) {
    geometryShader.use();
    
    // Use glUniform instead of setMat4
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, "model"), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, "view"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, "proj"), 1, GL_FALSE, &proj[0][0]);
}

Shader& DeferredRenderer::getGeometryShader() {
    return geometryShader;
}

Shader& DeferredRenderer::getLightingShader() {
    return lightingShader;
}