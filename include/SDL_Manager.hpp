#ifndef SDL_MANAGER_HPP
#define SDL_MANAGER_HPP

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>
#include <vector>
#include <string>
#include <glm/glm.hpp>

#define MAX_WINDOWS 10

class SDL_Manager {
private:
    SDL_Window* windows[MAX_WINDOWS];
    SDL_GLContext glContext;
    size_t count;

    // Constructor
    SDL_Manager();

    // Destructor
    ~SDL_Manager();

public:
    // Ensure singleton pattern
    SDL_Manager(const SDL_Manager&) = delete;
    SDL_Manager& operator=(const SDL_Manager&) = delete;

    // Accessor for singleton instance
    static SDL_Manager& sdl();

    // Close window by ID
    void closeWindow(uint32_t id);

    // Create new window
    void spawnWindow(const std::string& title, int width, int height, SDL_bool resizable = SDL_FALSE);

    // Update windows
    void updateWindows();
};

#endif // SDL_MANAGER_HPP