#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "GameObject.hpp"

class Renderer {
private:
    GLuint defaultTexture;  // A default white texture
    std::vector<GameObject*> renderQueue; // Queue of objects to render

    void initializeDefaultTexture(); // Creates a simple white texture
public:
    Renderer();
    ~Renderer();

    void submit(GameObject* object); // Add object to render queue
    void render(Shader& shader, const glm::mat4& view, const glm::mat4& proj);
};

#endif