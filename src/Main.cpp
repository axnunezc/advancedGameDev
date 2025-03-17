#include "SDL_Manager.hpp"
#include "Shape.hpp"
#include "Shader.hpp"
#include "Engine.hpp"
#include "GameObjectManager.hpp"
#include "GameObject.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
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
    return true;
}

int main(int argc, char** argv) {
    Engine::initialize();

    SDL_Manager& sdl = SDL_Manager::sdl();
    sdl.spawnWindow("OpenGL Cube", 800, 600, SDL_TRUE);

    if (!initOpenGL()) {
        return EXIT_FAILURE;
    }

    Shader shader("../shader.vert", "../shader.frag");

    std::vector<float> vertexData;
    size_t triangleCount;
    Shape* cubeShape = nullptr;

    if (!Shape::loadMeshData("../mesh.cse", triangleCount, vertexData)) {
        return EXIT_FAILURE;
    }

    Shape testObj(triangleCount, vertexData);

    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(60.0f), glm::normalize(glm::vec3(0.5f, 0.0f, 1.0f))); // Place cube at origin
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -10.0f)); // Move camera further back
    glm::mat4 proj = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    // glm::mat4 mvp = proj * view * model;
    GLint projLoc = shader.getUniform("proj");
    GLint modelLoc = shader.getUniform("model");
    GLint viewLoc = shader.getUniform("view");

    cubeShape = new Shape(triangleCount, vertexData);

    GameObjectManager virtualGameObjects;
    virtualGameObjects.addObject(new Cube(glm::vec3(0.0f, 0.0f, 0.0f), Quat(glm::vec3(0, 1, 0), 0.0f), cubeShape));

    GameObjectManager functionPtrGameObjects;

    Cube_FnPtr* cubeFnPtr = new Cube_FnPtr(glm::vec3(0.0f, 0.0f, 0.0f), Quat(glm::vec3(0, 1, 0), 0.0f), cubeShape);
    cubeFnPtr->setUpdateFunction(cubeUpdateFunction);  // Ensure the function pointer is set

    functionPtrGameObjects.addObject(cubeFnPtr);

    std::vector<double> virtualTimes;
    for (int i = 0; i < 10000; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        virtualGameObjects.updateAll(0.016f);
        auto end = std::chrono::high_resolution_clock::now();
        virtualTimes.push_back(std::chrono::duration<double, std::micro>(end - start).count());
    }

    std::vector<double> functionPtrTimes;
    for (int i = 0; i < 10000; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        functionPtrGameObjects.updateAll(0.016f);
        auto end = std::chrono::high_resolution_clock::now();
        functionPtrTimes.push_back(std::chrono::duration<double, std::micro>(end - start).count());
    }

    std::ofstream virtualFile("benchmark_virtual.csv");
    if (virtualFile.is_open()) {
        virtualFile << "Iteration,Update Time (µs)\n";
        for (size_t i = 0; i < virtualTimes.size(); ++i) {
            virtualFile << i << "," << virtualTimes[i] << "\n";
        }
        virtualFile.close();
        std::cout << "Virtual function benchmark results saved to benchmark_virtual.csv\n";
    } else {
        std::cerr << "Error: Could not open benchmark_virtual.csv for writing\n";
    }

    std::ofstream functionPtrFile("benchmark_function_ptr.csv");
    if (functionPtrFile.is_open()) {
        functionPtrFile << "Iteration,Update Time (µs)\n";
        for (size_t i = 0; i < functionPtrTimes.size(); ++i) {
            functionPtrFile << i << "," << functionPtrTimes[i] << "\n";
        }
        functionPtrFile.close();
        std::cout << "Function pointer benchmark results saved to benchmark_function_ptr.csv\n";
    } else {
        std::cerr << "Error: Could not open benchmark_function_ptr.csv for writing\n";
    }

    bool exit = false;
    SDL_Event e;

    while (!exit) {
        Engine::update();
        float dtSeconds = Engine::getDeltaSeconds();

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit = true;
            }
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_CLOSE) {
                    sdl.closeWindow(e.window.windowID);
                }
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    exit = true;
                }
            }
        }

        virtualGameObjects.updateAll(dtSeconds);
        functionPtrGameObjects.updateAll(dtSeconds);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.use();

        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(testObj.getVAO());
        glBindBuffer(GL_ARRAY_BUFFER, testObj.getVBO());

        glDrawArrays(GL_TRIANGLES, 0, testObj.getVertexCount());

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // for (size_t i = 0; i < virtualGameObjects.getCount(); ++i) {
        //     GameObject* obj = virtualGameObjects.getObject(i);
        //     if (obj) {
        //         glm::mat4 model = obj->getModel();
        //         glUniformMatrix4fv(shader.getUniform("model"), 1, GL_FALSE, glm::value_ptr(model));
        //         obj->draw();
        //     }
        // }

        sdl.updateWindows();
        // std::this_thread::sleep_for(1ms);
    }

    return 0;
}
