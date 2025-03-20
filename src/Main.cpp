#include "SDL_Manager.hpp"
#include "Shader.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "SoundSystem.hpp"
#include "Camera.hpp"
#include "Breakout.hpp"
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

// Render text - simplified version that just prints to console
void renderText(const std::string& text, float x, float y, float scale) {
    // In a real implementation, you'd use SDL_ttf or another text rendering solution
    // For now, just print to console
    std::cout << text << std::endl;
}

int main(int argc, char** argv) {
    // Initialize engine components
    Engine::initialize();
    SDL_Manager& sdl = SDL_Manager::sdl();
    
    // Create window
    const int windowWidth = 800;
    const int windowHeight = 600;
    sdl.spawnWindow("Breakout", windowWidth, windowHeight, SDL_TRUE);
    
    // Initialize OpenGL
    if (!initOpenGL()) return EXIT_FAILURE;
    
    // Set up shader
    Shader shader("../shader.vert", "../shader.frag");
    Renderer renderer;
    
    // Set up sound system
    SoundSystem soundSystem;
    
    // Try to load sound effects (continue even if they fail)
    soundSystem.loadSound("../audio/synth.wav");
    soundSystem.loadSound("../audio/brick_hit.wav");
    soundSystem.loadSound("../audio/wall_hit.wav");
    soundSystem.loadSound("../audio/lose_life.wav");
    
    // Set up orthographic camera for 2D rendering
    Camera camera;
    camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    camera.setNearPlane(0.1f);
    camera.setFarPlane(100.0f);
    
    // Use orthographic projection for 2D
    float aspectRatio = static_cast<float>(windowWidth) / windowHeight;
    glm::mat4 projection = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, 0.1f, 100.0f);
    
    // Create game boundaries in normalized coordinates
    float left = -aspectRatio;
    float right = aspectRatio;
    float bottom = -1.0f;
    float top = 1.0f;
    
    // Create Breakout game
    Breakout game(soundSystem, left, right, top, bottom);
    
    // Main game loop
    bool exit = false;
    SDL_Event e;
    
    // For input handling
    bool leftPressed = false;
    bool rightPressed = false;
    
    while (!exit) {
        // Update engine and delta time
        Engine::update();
        float dtSeconds = Engine::getDeltaSeconds();
        
        // Clean up completed sounds
        soundSystem.cleanup();
        
        // Process SDL events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit = true;
            }
            
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
                sdl.closeWindow(e.window.windowID);
                exit = true;
            }
            
            // Handle keyboard input
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        leftPressed = true;
                        break;
                    case SDLK_RIGHT:
                        rightPressed = true;
                        break;
                    case SDLK_SPACE:
                        // Space is handled in the handleInput function
                        break;
                    case SDLK_ESCAPE:
                        exit = true;
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch (e.key.keysym.sym) {
                    case SDLK_LEFT:
                        leftPressed = false;
                        break;
                    case SDLK_RIGHT:
                        rightPressed = false;
                        break;
                }
            }
        }
        
        // Handle input for the game
        game.handleInput(dtSeconds, leftPressed, rightPressed, SDL_GetKeyboardState(NULL)[SDL_SCANCODE_SPACE]);
        
        // Update game logic
        game.update(dtSeconds);
        
        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render game objects using the scene graph
        game.getSceneGraph()->render(renderer, camera);
        
        // Render using our shader
        renderer.render(shader, camera.getViewMatrix(), projection);
        
        // Render UI text based on game state
        if (game.getState() == GameState::Start) {
            // Start screen
            renderText("BREAKOUT", 0.0f, 0.2f, 1.0f);
            renderText("Press SPACE to start", 0.0f, 0.0f, 0.5f);
            renderText("Use LEFT/RIGHT to move paddle", 0.0f, -0.2f, 0.5f);
        } else if (game.getState() == GameState::GameOver) {
            // Game over screen
            renderText("GAME OVER", 0.0f, 0.2f, 1.0f);
            renderText("Final Score: " + std::to_string(game.getScore()), 0.0f, 0.0f, 0.5f);
            renderText("Press SPACE to restart", 0.0f, -0.2f, 0.5f);
        } else if (game.getState() == GameState::Win) {
            // Win screen
            renderText("YOU WIN!", 0.0f, 0.2f, 1.0f);
            renderText("Final Score: " + std::to_string(game.getScore()), 0.0f, 0.0f, 0.5f);
            renderText("Press SPACE to restart", 0.0f, -0.2f, 0.5f);
        } else {
            // In-game UI
            renderText("Score: " + std::to_string(game.getScore()), -aspectRatio + 0.1f, top - 0.1f, 0.5f);
            renderText("Lives: " + std::to_string(game.getLives()), aspectRatio - 0.3f, top - 0.1f, 0.5f);
        }
        
        // Update window
        sdl.updateWindows();
        
        // Cap frame rate
        std::this_thread::sleep_for(1ms);
    }
    
    return 0;
}