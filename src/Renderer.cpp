#include "Renderer.hpp"
#include <iostream>

Renderer::Renderer() {
    initializeDefaultTexture();
}

Renderer::~Renderer() {
    glDeleteTextures(1, &defaultTexture);
}

void Renderer::initializeDefaultTexture() {
    glGenTextures(1, &defaultTexture);
    glBindTexture(GL_TEXTURE_2D, defaultTexture);

    unsigned char whitePixel[3] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Adds a GameObject to the render queue
void Renderer::submit(GameObject* object) {
    renderQueue.push_back(object);
}

// Draws all objects
void Renderer::render(Shader& shader, const glm::mat4& view, const glm::mat4& proj) {
    shader.use();
    GLint projLoc = shader.getUniform("proj");
    GLint viewLoc = shader.getUniform("view");
    
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    for (auto object : renderQueue) {
        // Bind the object's VAO and VBO for rendering
        glBindVertexArray(object->getVAO());
        glBindBuffer(GL_ARRAY_BUFFER, object->getVBO());

        // Set model matrix uniform
        glm::mat4 model = object->getModelMatrix();
        GLint modelLoc = shader.getUniform("model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Check if object has an armature and set appropriate uniforms
        if (object->hasAnimatableSkeleton()) {
            // Set bone matrices in shader if the uniform exists
            GLint hasArmatureLoc = shader.getUniform("hasArmature");
            if (hasArmatureLoc != -1) {
                glUniform1i(hasArmatureLoc, GL_TRUE);
                
                // Get bone count and matrices
                const auto& boneMatrices = object->getBoneMatrices();
                GLint boneCountLoc = shader.getUniform("boneCount");
                if (boneCountLoc != -1) {
                    glUniform1i(boneCountLoc, static_cast<int>(boneMatrices.size()));
                }
                
                // Upload bone matrices to shader
                for (size_t i = 0; i < boneMatrices.size(); i++) {
                    std::string uniformName = "boneMatrices[" + std::to_string(i) + "]";
                    GLint matrixLoc = shader.getUniform(uniformName.c_str());
                    if (matrixLoc != -1) {
                        glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, glm::value_ptr(boneMatrices[i]));
                    }
                }
            }
        } else {
            // Set hasArmature to false if the uniform exists
            GLint hasArmatureLoc = shader.getUniform("hasArmature");
            if (hasArmatureLoc != -1) {
                glUniform1i(hasArmatureLoc, GL_FALSE);
            }
        }

        // Draw the object
        glDrawArrays(GL_TRIANGLES, 0, object->getVertexCount());
        
        // Clean up
        glBindVertexArray(0);
    }

    renderQueue.clear();
}