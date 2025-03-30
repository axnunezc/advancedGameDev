#include "SDL_Manager.hpp"
#include "Shader.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
#include "SoundSystem.hpp"
#include "Camera.hpp"
#include "Breakout.hpp"
#include "PhysicsIntegrator.hpp"
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

// Helper function to create a simple test shape for physics
Shape createTestShape() {
    // Quad vertices data (all positions first, then all normals)
    std::vector<float> vertexData;
    
    // First triangle - positions
    vertexData.push_back(-0.5f); vertexData.push_back(-0.5f); vertexData.push_back(0.0f);
    vertexData.push_back(0.5f);  vertexData.push_back(-0.5f); vertexData.push_back(0.0f);
    vertexData.push_back(0.5f);  vertexData.push_back(0.5f);  vertexData.push_back(0.0f);
    
    // Second triangle - positions
    vertexData.push_back(-0.5f); vertexData.push_back(-0.5f); vertexData.push_back(0.0f);
    vertexData.push_back(0.5f);  vertexData.push_back(0.5f);  vertexData.push_back(0.0f);
    vertexData.push_back(-0.5f); vertexData.push_back(0.5f);  vertexData.push_back(0.0f);
    
    // Now add all normals (6 vertices)
    for (int i = 0; i < 6; i++) {
        vertexData.push_back(0.0f); vertexData.push_back(0.0f); vertexData.push_back(1.0f);
    }
    
    return Shape(2, vertexData); // 2 triangles
}

// Function to test physics integration with a simple GameObject
void testPhysics() {
    std::cout << "Testing physics integration..." << std::endl;
    
    // Create a test shape
    Shape testShape = createTestShape();
    
    // Create a test GameObject
    GameObject testObj(
        glm::vec3(0.0f, 0.0f, 0.0f),         // Initial position
        Quaternion(0.0f, glm::vec3(0.0f, 0.0f, 1.0f)), // Initial rotation
        testShape,
        999 // ID
    );
    
    // Set up physics properties
    testObj.setVelocity(glm::vec3(1.0f, 0.0f, 0.0f));  // Moving in X direction
    testObj.setAngularVelocity(glm::vec3(0.0f, 0.0f, 45.0f)); // Rotating around Z
    
    // Print initial state
    std::cout << "Initial position: (" 
              << testObj.getPosition().x << ", "
              << testObj.getPosition().y << ", "
              << testObj.getPosition().z << ")" << std::endl;
    
    std::cout << "Initial rotation angle: " << testObj.getRotation().getAngle() << " degrees" << std::endl;
    
    // Simulate 1 second of physics updates (10 steps of 0.1 seconds)
    float dt = 0.1f;
    for (int i = 0; i < 10; i++) {
        Physics::updateObject(&testObj, dt, false); // No gravity
        
        std::cout << "Step " << (i+1) << " position: (" 
                  << testObj.getPosition().x << ", "
                  << testObj.getPosition().y << ", "
                  << testObj.getPosition().z << ")" << std::endl;
        
        std::cout << "Step " << (i+1) << " rotation angle: " 
                  << testObj.getRotation().getAngle() << " degrees" << std::endl;
    }
    
    // Test acceleration
    std::cout << "\nTesting acceleration..." << std::endl;
    
    // Reset object
    testObj.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    testObj.setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
    testObj.setRotation(Quaternion(0.0f, glm::vec3(0.0f, 0.0f, 1.0f)));
    testObj.setAngularVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Apply constant acceleration (like gravity)
    glm::vec3 accel(0.0f, -9.81f, 0.0f);
    
    for (int i = 0; i < 10; i++) {
        Physics::integrateAcceleration(&testObj, dt, accel);
        Physics::updateObject(&testObj, dt, false); // Don't apply gravity again
    }
    
    std::cout << "After 1 second with gravity:" << std::endl;
    std::cout << "Position: (" 
              << testObj.getPosition().x << ", "
              << testObj.getPosition().y << ", "
              << testObj.getPosition().z << ")" << std::endl;
    std::cout << "Velocity: (" 
              << testObj.getVelocity().x << ", "
              << testObj.getVelocity().y << ", "
              << testObj.getVelocity().z << ")" << std::endl;
    
    // Test impulse
    std::cout << "\nTesting impulse..." << std::endl;
    
    // Reset object
    testObj.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
    testObj.setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Apply impulse
    glm::vec3 impulse(3.0f, 4.0f, 0.0f);
    Physics::applyLinearImpulse(&testObj, impulse);
    
    std::cout << "After impulse, velocity: (" 
              << testObj.getVelocity().x << ", "
              << testObj.getVelocity().y << ", "
              << testObj.getVelocity().z << ")" << std::endl;
    
    // Update position for 1 second
    for (int i = 0; i < 10; i++) {
        Physics::updateObject(&testObj, dt, false);
    }
    
    std::cout << "Final position after impulse: (" 
              << testObj.getPosition().x << ", "
              << testObj.getPosition().y << ", "
              << testObj.getPosition().z << ")" << std::endl;
    
    std::cout << "Physics testing complete." << std::endl;
}

int main(int argc, char** argv) {
    
    // Your normal engine initialization code
    Engine::initialize();
    
    SDL_Manager& sdl = SDL_Manager::sdl();
    sdl.spawnWindow("Physics Test", 800, 600, SDL_TRUE);

    if (!initOpenGL()) return EXIT_FAILURE;

    // Test physics integration first
    testPhysics();
    
    // Initialize sound system
    SoundSystem soundSystem;

    // Set background color to very dark blue
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    
    // Load shader
    Shader shader("../shader.vert", "../shader.frag");
    Renderer renderer;
    
    // Create camera
    Camera camera(60.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Create a test object to visualize physics
    Shape testShape = createTestShape();
    GameObject testObj(
        glm::vec3(0.0f, 0.0f, 0.0f),
        Quaternion(0.0f, glm::vec3(0.0f, 0.0f, 1.0f)),
        testShape,
        1
    );
    
    // Set initial velocity
    testObj.setVelocity(glm::vec3(1.0f, 0.0f, 0.0f));
    testObj.setAngularVelocity(glm::vec3(0.0f, 0.0f, 45.0f));
    
    bool exit = false;
    SDL_Event e;
    
    while (!exit) {
        Engine::update();
        float dt = Engine::getDeltaSeconds();
        
        // Process events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit = true;
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
                sdl.closeWindow(e.window.windowID);
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                exit = true;
            }
        }

        // Update physics for test object
        Physics::updateObject(&testObj, dt, false);

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render the test object
        renderer.submit(&testObj);
        
        // Render using the renderer
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix();
        renderer.render(shader, view, proj);
        
        // Update window
        sdl.updateWindows();
        std::this_thread::sleep_for(16ms);
        
        // Print object state
        static float printTimer = 0.0f;
        printTimer += dt;
        if (printTimer >= 0.5f) {
            std::cout << "Position: (" 
                      << testObj.getPosition().x << ", "
                      << testObj.getPosition().y << ", "
                      << testObj.getPosition().z << ")" << std::endl;
            std::cout << "Rotation: " << testObj.getRotation().getAngle() << " degrees" << std::endl;
            printTimer = 0.0f;
        }
    }

    return 0;
}