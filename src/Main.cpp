#include "SDL_Manager.hpp"
#include "Shader.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
#include "SoundSystem.hpp"
#include "SceneGraph.hpp"
#include "Camera.hpp"
#include "PhysicsIntegrator.hpp"
#include "GJK.hpp"
#include "EnhancedSceneGraph.hpp"
#include "Framebuffer.hpp"
#include "QuadRenderer.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath> // For sin and cos functions
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <SDL.h>

using namespace std::chrono_literals;

// Maximum number of lights (should match shader definition)
#define MAX_LIGHTS 16

bool initOpenGL() {
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Initialization Failed: " << glewGetErrorString(err) << std::endl;
        return false;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    return true;
}

// Helper function to create a cube shape
Shape createCubeShape() {
    std::vector<float> vertexData;
    
    // Front face (position, then normals)
    vertexData.push_back(-0.5f); vertexData.push_back(-0.5f); vertexData.push_back(0.5f);
    vertexData.push_back(0.5f);  vertexData.push_back(-0.5f); vertexData.push_back(0.5f);
    vertexData.push_back(0.5f);  vertexData.push_back(0.5f);  vertexData.push_back(0.5f);
    
    vertexData.push_back(-0.5f); vertexData.push_back(-0.5f); vertexData.push_back(0.5f);
    vertexData.push_back(0.5f);  vertexData.push_back(0.5f);  vertexData.push_back(0.5f);
    vertexData.push_back(-0.5f); vertexData.push_back(0.5f);  vertexData.push_back(0.5f);
    
    // Add normals for front face
    for (int i = 0; i < 6; i++) {
        vertexData.push_back(0.0f); vertexData.push_back(0.0f); vertexData.push_back(1.0f);
    }
    
    return Shape(2, vertexData);
}

// Helper function to calculate light position in an orbit
glm::vec3 calculateOrbitPosition(float radius, float angle, float height) {
    return glm::vec3(
        radius * std::cos(angle),
        height,
        radius * std::sin(angle)
    );
}

class TestCube : public GameObject {
    public:
        TestCube(const glm::vec3& pos, const Shape& shape) 
            : GameObject(pos, Quaternion(0.0f, glm::vec3(0.0f, 1.0f, 0.0f)), shape, 1) {}
        
        // Override getTypeId to return a fixed value
        int getTypeId() const override { return 1; }
};

class Armature : public GameObject {
    public:
        Armature(const glm::vec3& pos, const Shape& shape) 
            : GameObject(pos, Quaternion(90.0f, glm::vec3(1.0f, 0.0f, 0.0f)), shape, 2) {}
        
        // Override getTypeId to return a fixed value
        int getTypeId() const override { return 2; }
};

// Light object (small cube representing a light)
class LightObject : public GameObject {
    public:
        LightObject(const glm::vec3& pos, const Shape& shape, const glm::vec3& color) 
            : GameObject(pos, Quaternion(0.0f, glm::vec3(0.0f, 1.0f, 0.0f)), shape, 3),
              lightColor(color) {}
        
        // Override getTypeId to return a fixed value
        int getTypeId() const override { return 3; }
        
        // Get light color
        const glm::vec3& getLightColor() const { return lightColor; }
        
    private:
        glm::vec3 lightColor;
};

int main(int argc, char** argv) {
    // Engine initialization
    Engine::initialize();
    
    // Window dimensions
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;
    
    SDL_Manager& sdl = SDL_Manager::sdl();
    sdl.spawnWindow("Deferred Rendering with N-Lights", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_TRUE);

    if (!initOpenGL()) return EXIT_FAILURE;
    
    // Initialize sound system
    SoundSystem soundSystem;

    // Set background color to dark gray
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    
    // Create the G-Buffer
    std::vector<TextureProperties> textureProps;
    textureProps.push_back(TextureProperties(GL_RGB32F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST));  // Diffuse
    textureProps.push_back(TextureProperties(GL_RGB32F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST));  // Normal
    textureProps.push_back(TextureProperties(GL_RGB32F, GL_RGB, GL_FLOAT, GL_NEAREST, GL_NEAREST));  // Position
    Framebuffer gBuffer(WINDOW_WIDTH, WINDOW_HEIGHT, textureProps, true);
    
    // Create geometry pass shader
    Shader geometryShader("../deferred.vert", "../deferred.frag");
    
    // Create lighting shader for deferred rendering
    Shader lightingShader("../deferred_display.vert", "../lighting.frag");
    
    // Create shader for displaying individual G-Buffer textures
    Shader displayShader("../deferred_display.vert", "../deferred_display.frag");
    
    // Create quad renderer for screen rendering
    QuadRenderer quadRenderer;
    
    // Create camera
    Camera camera(60.0f, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
    camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Create an enhanced scene graph with large world bounds
    AABB worldBounds(glm::vec3(-100.0f), glm::vec3(100.0f));
    EnhancedSceneGraph sceneGraph(worldBounds);
    
    // Set collision method to GJK
    sceneGraph.setCollisionMethod(EnhancedSceneGraph::GJK);
    
    // Create a cube shape
    Shape cubeShape = createCubeShape();
    
    // Create a smaller cube shape for lights
    Shape lightShape = createCubeShape();

    // Load the armature mesh
    size_t vertexCount;
    size_t faceCount;
    std::vector<float> positionData;
    std::vector<float> normalData; 
    std::vector<float> uvData;
    std::vector<Bone> bones;
    std::vector<VertexBoneData> vertexBoneData;
    bool hasBones;

    if (!loadMeshWithArmature("../suzanne.mesh", vertexCount, faceCount, positionData, normalData, uvData, bones, vertexBoneData, hasBones)) {
        std::cerr << "Failed to load armature mesh!" << std::endl;
        return EXIT_FAILURE;
    }

    Shape armatureShape(faceCount, positionData, normalData, uvData, bones, vertexBoneData, hasBones);
    
    // Create game objects
    TestCube* cube = new TestCube(glm::vec3(3.0f, 0.0f, 0.1f), cubeShape);
    Armature* armature = new Armature(glm::vec3(0.0f, 0.0f, 0.0f), armatureShape);
    
    // Scale the armature to be more visible
    armature->setScale(glm::vec3(5.0f, 5.0f, 5.0f));
    
    // Update bounding boxes
    armature->updateBoundingBox();
    cube->updateBoundingBox();
    
    // Add objects to scene graph
    sceneGraph.addObject(cube);
    sceneGraph.addObject(armature);
    
    // Create light objects with different colors
    std::vector<LightObject*> lightObjects;
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;
    
    // Define some light colors
    glm::vec3 colors[] = {
        glm::vec3(1.0f, 0.2f, 0.2f),  // Red
        glm::vec3(0.2f, 1.0f, 0.2f),  // Green
        glm::vec3(0.2f, 0.2f, 1.0f),  // Blue
        glm::vec3(1.0f, 1.0f, 0.2f)   // Yellow
    };
    
    // Create four lights in different positions
    for (int i = 0; i < 4; i++) {
        // Initial position in a circle around the scene
        float angle = i * (glm::pi<float>() * 0.5f); // Distribute lights in a circle
        float radius = 7.0f;
        float height = 2.0f + i * 0.5f; // Vary height
        
        glm::vec3 position = calculateOrbitPosition(radius, angle, height);
        
        // Create light object
        LightObject* light = new LightObject(position, lightShape, colors[i]);
        light->setScale(glm::vec3(0.3f)); // Make light objects small
        
        // Add to collections
        lightObjects.push_back(light);
        lightPositions.push_back(position);
        lightColors.push_back(colors[i]);
        
        // Add to scene graph
        sceneGraph.addObject(light);
    }
    
    // Register collision callback
    sceneGraph.getResponder().registerCallback(cube->getTypeId(), armature->getTypeId(), 
        [](GameObject* a, GameObject* b) {
            std::cout << "GJK COLLISION DETECTED BETWEEN CUBE AND ARMATURE!" << std::endl;
            
            // Determine which object is the cube (type ID 1)
            GameObject* cubeObj = (a->getTypeId() == 1) ? a : b;
            
            // Reverse direction and increase speed
            cubeObj->setVelocity(-cubeObj->getVelocity() * 1.5f);
            
            // Print current positions for debugging
            std::cout << "Cube position: (" << cubeObj->getPosition().x << ", " 
                      << cubeObj->getPosition().y << ", " 
                      << cubeObj->getPosition().z << ")" << std::endl;
                      
            // Print current velocities for debugging
            std::cout << "Cube velocity: (" << cubeObj->getVelocity().x << ", " 
                      << cubeObj->getVelocity().y << ", " << cubeObj->getVelocity().z << ")" << std::endl;
        }
    );
    
    // Set initial velocity for cube (move toward center)
    cube->setVelocity(glm::vec3(-2.0f, 0.0f, 0.0f));
    
    // Variables for armature rotation
    float rotationSpeed = 30.0f;  // degrees per second
    float currentRotation = 0.0f;
    
    // Variables for light movement
    float lightOrbitSpeed = 0.5f; // radians per second
    float totalTime = 0.0f;
    
    // Display mode for G-Buffer visualization
    // 0 = Full deferred rendering (combined result)
    // 1 = Diffuse buffer only
    // 2 = Normal buffer only
    // 3 = Position buffer only
    int displayMode = 0;
    
    bool exit = false;
    SDL_Event e;
    
    std::cout << "Deferred Rendering with N-Lights Demo." << std::endl;
    std::cout << "Press SPACE to reset cube position." << std::endl;
    std::cout << "Press V to cycle through view modes (Combined, Diffuse, Normal, Position)" << std::endl;
    
    while (!exit) {
        Engine::update();
        float dt = Engine::getDeltaSeconds();
        totalTime += dt;
        
        // Process events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit = true;
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
                sdl.closeWindow(e.window.windowID);
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                exit = true;
            }
            
            // Reset cube position with spacebar
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                cube->setPosition(glm::vec3(3.0f, 0.0f, -0.3f));  // Match Z with armature
                cube->setVelocity(glm::vec3(-2.0f, 0.0f, 0.0f));
                std::cout << "Cube position reset. Moving toward armature again." << std::endl;
            }
            
            // Toggle collision detection method with 'G' key
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_g) {
                if (sceneGraph.getCollisionMethod() == EnhancedSceneGraph::GJK) {
                    sceneGraph.setCollisionMethod(EnhancedSceneGraph::MPR);
                    std::cout << "Switched to MPR collision detection" << std::endl;
                } else {
                    sceneGraph.setCollisionMethod(EnhancedSceneGraph::GJK);
                    std::cout << "Switched to GJK collision detection" << std::endl;
                }
            }
            
            // Toggle display mode with 'V' key
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_v) {
                displayMode = (displayMode + 1) % 4;
                std::cout << "DISPLAY MODE CHANGED TO: " << displayMode << std::endl;
                switch (displayMode) {
                    case 0: std::cout << "Display mode: Combined result" << std::endl; break;
                    case 1: std::cout << "Display mode: Diffuse buffer" << std::endl; break;
                    case 2: std::cout << "Display mode: Normal buffer" << std::endl; break;
                    case 3: std::cout << "Display mode: Position buffer" << std::endl; break;
                }
            }
        }
        
        // Update armature rotation
        currentRotation += rotationSpeed * dt;
        if (currentRotation > 360.0f) {
            currentRotation -= 360.0f;
        }
        
        Quaternion combinedRotation;

        // Combine rotations
        Quaternion xRotation(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        Quaternion yRotation(currentRotation, glm::vec3(0.0f, 1.0f, 0.0f));
        combinedRotation = yRotation * xRotation;  // Apply X rotation first, then Y rotation

        // Apply the combined rotation directly
        armature->setRotation(combinedRotation);
        
        // Update light positions (orbit around the scene)
        for (size_t i = 0; i < lightObjects.size(); i++) {
            // Calculate new position
            float angle = totalTime * lightOrbitSpeed + i * (glm::pi<float>() * 0.5f);
            float radius = 7.0f + std::sin(totalTime * 0.3f + i) * 1.0f; // Vary radius
            float height = 2.0f + i * 0.5f + std::sin(totalTime * 0.5f + i) * 1.0f; // Vary height
            
            glm::vec3 position = calculateOrbitPosition(radius, angle, height);
            
            // Update light object
            lightObjects[i]->setPosition(position);
            
            // Update position in light positions array
            lightPositions[i] = position;
        }

        // Update physics
        Physics::updateObject(cube, dt, false);
        Physics::updateObject(armature, dt, false);
        
        // Force update bounding boxes to ensure collision detection works
        cube->updateBoundingBox();
        armature->updateBoundingBox();
        
        // Update scene graph and process collisions
        sceneGraph.updateSpatialStructure(dt);
        sceneGraph.processCollisionResponses();
        
        // Get view and projection matrices
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix();
        
        // FIRST PASS: Geometry pass to G-Buffer
        // Bind the G-Buffer framebuffer
        gBuffer.bindFBO();
        
        // Clear the G-Buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use the geometry shader
        geometryShader.use();
        
        // Set common uniforms
        glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, "proj"), 1, GL_FALSE, glm::value_ptr(proj));
        
        // Render cube
        glUniform1i(glGetUniformLocation(geometryShader.program, "hasTexture"), 0);
        glUniform1i(glGetUniformLocation(geometryShader.program, "hasArmature"), 0);
        glUniform3f(glGetUniformLocation(geometryShader.program, "baseColor"), 0.9f, 0.3f, 0.3f);
        glUniform4f(glGetUniformLocation(geometryShader.program, "objectColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        
        glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, "model"), 1, GL_FALSE, 
                          glm::value_ptr(cube->getModelMatrix()));
        
        glBindVertexArray(cube->getVAO());
        glDrawArrays(GL_TRIANGLES, 0, cube->getVertexCount());
        
        // Render armature
        glUniform3f(glGetUniformLocation(geometryShader.program, "baseColor"), 0.3f, 0.7f, 0.9f);
        
        // Set armature-specific uniforms (bone matrices if available)
        if (armature->hasAnimatableSkeleton()) {
            glUniform1i(glGetUniformLocation(geometryShader.program, "hasArmature"), 1);
            glUniform1i(glGetUniformLocation(geometryShader.program, "boneCount"), 
                       static_cast<int>(armature->getBoneMatrices().size()));
            
            // Set bone matrices
            for (size_t i = 0; i < armature->getBoneMatrices().size(); i++) {
                std::string uniformName = "boneMatrices[" + std::to_string(i) + "]";
                glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, uniformName.c_str()), 
                                 1, GL_FALSE, glm::value_ptr(armature->getBoneMatrices()[i]));
            }
        } else {
            glUniform1i(glGetUniformLocation(geometryShader.program, "hasArmature"), 0);
        }
        
        glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, "model"), 1, GL_FALSE, 
                          glm::value_ptr(armature->getModelMatrix()));
        
        glBindVertexArray(armature->getVAO());
        glDrawArrays(GL_TRIANGLES, 0, armature->getVertexCount());
        
        // Render light objects (small cubes representing lights)
        for (size_t i = 0; i < lightObjects.size(); i++) {
            // Set light color as base color
            glUniform3fv(glGetUniformLocation(geometryShader.program, "baseColor"), 1, 
                        glm::value_ptr(lightObjects[i]->getLightColor()));
            
            // Set model matrix
            glUniformMatrix4fv(glGetUniformLocation(geometryShader.program, "model"), 1, GL_FALSE, 
                              glm::value_ptr(lightObjects[i]->getModelMatrix()));
            
            // Draw light cube
            glBindVertexArray(lightObjects[i]->getVAO());
            glDrawArrays(GL_TRIANGLES, 0, lightObjects[i]->getVertexCount());
        }
        
        // SECOND PASS: Display G-Buffer information
        // Bind default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        
        // Clear default framebuffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (displayMode == 0) {
            // Combined result with lighting (use lighting shader)
            lightingShader.use();
            
            // Bind G-Buffer textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(0)); // Diffuse
            glUniform1i(glGetUniformLocation(lightingShader.program, "diffuseTexture"), 0);
            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(1)); // Normal
            glUniform1i(glGetUniformLocation(lightingShader.program, "normalTexture"), 1);
            
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(2)); // Position
            glUniform1i(glGetUniformLocation(lightingShader.program, "positionTexture"), 2);
            
            // Set camera position for specular calculations
            glUniform3fv(glGetUniformLocation(lightingShader.program, "viewPos"), 1, 
                        glm::value_ptr(camera.getPosition()));
            
            // Set ambient light
            glUniform3f(glGetUniformLocation(lightingShader.program, "ambientColor"), 0.1f, 0.1f, 0.1f);
            
            // Set attenuation factors
            glUniform1f(glGetUniformLocation(lightingShader.program, "constantFactor"), 0.1f);
            glUniform1f(glGetUniformLocation(lightingShader.program, "linearFactor"), 0.01f);
            glUniform1f(glGetUniformLocation(lightingShader.program, "quadraticFactor"), 0.001f);
            
            // Set light count
            glUniform1i(glGetUniformLocation(lightingShader.program, "numActiveLights"), 
                       static_cast<int>(lightPositions.size()));
            
            // Set light positions and colors
            for (size_t i = 0; i < lightPositions.size(); i++) {
                std::string posName = "lightPositions[" + std::to_string(i) + "]";
                std::string colorName = "lightColors[" + std::to_string(i) + "]";
                
                glUniform3fv(glGetUniformLocation(lightingShader.program, posName.c_str()), 1, 
                            glm::value_ptr(lightPositions[i]));
                glUniform3fv(glGetUniformLocation(lightingShader.program, colorName.c_str()), 1, 
                            glm::value_ptr(lightColors[i]));
            }
        } else {
            // Display individual G-Buffer (use display shader)
            displayShader.use();
            
            // Bind G-Buffer textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(0)); // Diffuse
            glUniform1i(glGetUniformLocation(displayShader.program, "diffuseTexture"), 0);
            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(1)); // Normal
            glUniform1i(glGetUniformLocation(displayShader.program, "normalTexture"), 1);
            
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gBuffer.getTexture(2)); // Position
            glUniform1i(glGetUniformLocation(displayShader.program, "positionTexture"), 2);
            
            // Set display mode
            glUniform1i(glGetUniformLocation(displayShader.program, "displayMode"), displayMode);
        }
        
        // Render quad
        quadRenderer.renderQuad();
        
        // Update window
        sdl.updateWindows();
        std::this_thread::sleep_for(16ms);
    }

    // Clean up
    delete cube;
    delete armature;
    
    // Clean up light objects
    for (LightObject* light : lightObjects) {
        delete light;
    }
    lightObjects.clear();

    return 0;
}