#include "SDL_Manager.hpp"
#include "Shader.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
#include "RotatingCube.hpp"
#include "SoundSystem.hpp"
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
    sdl.spawnWindow("OpenGL Cube", 800, 600, SDL_TRUE);

    if (!initOpenGL()) return EXIT_FAILURE;

    Shader shader("../shader.vert", "../shader.frag");
    Renderer renderer;
    SoundSystem soundSystem;

    // Load and play the initial sound
    int soundIndex = soundSystem.loadSound("../audio/hit.wav");
    std::cout << "Loaded sound index: " << soundIndex << std::endl;
    if (soundIndex == -1) {
        std::cerr << "Failed to load initial sound.\n";
    } else {
        std::cout << "Playing initial sound..." << std::endl;
        soundSystem.playSound(soundIndex);
    }

    // Load mesh
    size_t triangleCount;
    std::vector<float> vertexData;
    if (!loadMeshData("../mesh.cse", triangleCount, vertexData)) return EXIT_FAILURE;

    Shape cubeShape(triangleCount, vertexData);

    // Create a RotatingCube with a rotation speed and axis
    RotatingCube cube(
        glm::vec3(0.0f, 0.0f, 0.0f),    // Position
        Quaternion(60.0f, glm::vec3(0.5f, 0.0f, 1.0f)), // Initial rotation
        cubeShape,                       // Shape
        1,                                // ID
        glm::vec3(0.5f, 0.0f, 1.0f),      // Rotation Axis
        90.0f                             // Rotation Speed (Degrees per second)
    );    

    // Projection and View Matrices
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f));
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    bool exit = false;
    SDL_Event e;

    while (!exit) {
        Engine::update();
        float dtSeconds = Engine::getDeltaSeconds();
        
        // Clean up completed sounds to prevent memory issues
        soundSystem.cleanup();
        
        cube.update(dtSeconds);
        renderer.submit(&cube);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit = true;
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
                sdl.closeWindow(e.window.windowID);
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    std::cout << "Playing sound on SPACEBAR press...\n";
                    soundSystem.playSound(soundIndex);  // Play the loaded sound
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.render(shader, view, proj);
        sdl.updateWindows();
        std::this_thread::sleep_for(1ms);
    }

    return 0;
}
