#include "SDL_Manager.hpp"
#include "Shader.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
#include "RotatingCube.hpp"
#include "SoundSystem.hpp"
#include "SceneGraph.hpp"
#include "Camera.hpp"
#include <iostream>
#include <vector>
#include <thread> 
#include <chrono> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <SDL.h>

using namespace std::chrono_literals;

// OpenGL Initialization
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

int main(int argc, char** argv) {
    Engine::initialize();
    SDL_Manager& sdl = SDL_Manager::sdl();
    sdl.spawnWindow("OpenGL Scene", 800, 600, SDL_TRUE);

    if (!initOpenGL()) return EXIT_FAILURE;

    Shader shader("../shader.vert", "../shader.frag");
    Renderer renderer;
    SoundSystem soundSystem;

    // Load and play the initial sound
    int soundIndex = soundSystem.loadSound("../audio/synth.wav");
    if (soundIndex == -1) {
        std::cerr << "Failed to load initial sound.\n";
    } else {
        soundSystem.playSound(soundIndex);  // Play the loaded sound
    }

    // Load mesh
    size_t triangleCount;
    std::vector<float> vertexData;
    if (!loadMeshData("../mesh.cse", triangleCount, vertexData)) return EXIT_FAILURE;

    Shape cubeShape(triangleCount, vertexData);
    
    // Create camera
    Camera camera(60.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    camera.setPosition(glm::vec3(0.0f, 5.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Initialize Scene Graph with a large world bounds
    AABB worldBounds(glm::vec3(-100.0f, -100.0f, -100.0f), glm::vec3(100.0f, 100.0f, 100.0f));
    SceneGraph sceneGraph(worldBounds);
    
    // Create a bunch of cubes in the scene
    std::vector<std::unique_ptr<RotatingCube>> cubes;
    
    // Create parent node for a group of cubes
    SceneNode* parentNode = sceneGraph.createNode();
    parentNode->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f)));
    
    // Add center cube
    auto centerCube = std::make_unique<RotatingCube>(
        glm::vec3(0.0f, 0.0f, 0.0f),    // Position
        Quaternion(0.0f, glm::vec3(0.0f, 1.0f, 0.0f)), // No initial rotation
        cubeShape,                       // Shape
        1,                                // ID
        glm::vec3(0.0f, 1.0f, 0.0f),      // Rotation Axis (Y-axis)
        45.0f                             // Rotation Speed (Degrees per second)
    );
    
    // Add cube to scene graph (both hierarchy and spatial structure)
    sceneGraph.addObject(centerCube.get(), sceneGraph.getRootNode());
    cubes.push_back(std::move(centerCube));
    
    // Create a ring of cubes around the center
    const int numCubes = 8;
    const float radius = 5.0f;
    
    for (int i = 0; i < numCubes; i++) {
        float angle = (float)i / numCubes * 2.0f * 3.14159f;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        
        // Create child node
        SceneNode* childNode = sceneGraph.createNode(parentNode);
        childNode->setLocalTransform(glm::translate(glm::mat4(1.0f), glm::vec3(x, 0.0f, z)));
        
        // Create cube with unique rotation axis
        auto cube = std::make_unique<RotatingCube>(
            glm::vec3(0.0f, 0.0f, 0.0f),    // Local position (will be transformed by node)
            Quaternion(angle * 57.3f, glm::vec3(0.0f, 1.0f, 0.0f)), // Initial rotation
            cubeShape,                       // Shape
            i + 2,                           // ID
            glm::normalize(glm::vec3(cos(angle), 1.0f, sin(angle))),  // Unique rotation axis
            60.0f + i * 5.0f                 // Varying rotation speed
        );
        
        // Add cube to scene graph
        sceneGraph.addObject(cube.get(), childNode);
        cubes.push_back(std::move(cube));
    }
    
    // Update scene graph transformations
    sceneGraph.updateTransforms();
    
    bool exit = false;
    SDL_Event e;
    
    float cameraRotation = 0.0f;

    while (!exit) {
        Engine::update();
        float dtSeconds = Engine::getDeltaSeconds();
        
        // Clean up completed sounds to prevent memory issues
        soundSystem.cleanup();
        
        // Update all cubes
        for (auto& cube : cubes) {
            cube->update(dtSeconds);
        }
        
        // Rotate camera around the scene
        cameraRotation += dtSeconds * 0.2f; // Slow rotation
        float camX = sin(cameraRotation) * 15.0f;
        float camZ = cos(cameraRotation) * 15.0f;
        camera.setPosition(glm::vec3(camX, 7.0f, camZ));
        camera.updateMatrices();
        
        // Update scene graph spatial structure (in a real game, you might do this less frequently)
        sceneGraph.updateSpatialStructure();
        
        // Process events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit = true;
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
                sdl.closeWindow(e.window.windowID);
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    std::cout << "Playing sound on SPACEBAR press...\n";
                    soundSystem.playSound(soundIndex);
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render using the scene graph for efficient culling
        sceneGraph.render(renderer, camera);
        
        // Render the collected objects
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix();
        renderer.render(shader, view, proj);
        
        sdl.updateWindows();
        std::this_thread::sleep_for(1ms);
    }

    return 0;
}