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
        glBindVertexArray(object->getVAO());
        glBindBuffer(GL_ARRAY_BUFFER, object->getVBO());

        glm::mat4 model = object->getModelMatrix();

        GLint modelLoc = shader.getUniform("model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glDrawArrays(GL_TRIANGLES, 0, object->getVertexCount());

        glBindVertexArray(0);
    }

    renderQueue.clear();
}
