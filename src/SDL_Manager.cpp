#include "SDL_Manager.hpp"

SDL_Manager::SDL_Manager() : count(0) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("Failed to initialize SDL: " + std::string(SDL_GetError()));
    }
}

SDL_Manager::~SDL_Manager() {
    if (glContext) {
        SDL_GL_DeleteContext(glContext);
        glContext = nullptr;
    }

    for (int i = 0; i < count; ++i) {
        if (windows[i]) SDL_DestroyWindow(windows[i]);
    }

    SDL_Quit();
}

SDL_Manager& SDL_Manager::sdl() {
    static SDL_Manager instance;
    return instance;
}

void SDL_Manager::spawnWindow(const std::string& title, int width, int height, SDL_bool resizable) {
    if (count >= MAX_WINDOWS) {
        std::cerr << "Maximum number of windows reached. Cannot create more windows.\n";
        return;
    }

    uint32_t flags = SDL_WINDOW_SHOWN | ((count == 0) ? SDL_WINDOW_OPENGL : 0);
    if (resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }

    // Create a new window
    SDL_Window* newWindow = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        flags
    );

    // Check window creation
    if (!newWindow) {
        std::cerr << "Failed to create window: " << SDL_GetError() << "\n";
        exit(EXIT_FAILURE);
    }

    // Bring window to the foreground and request focus
    SDL_RaiseWindow(newWindow);
    SDL_SetWindowInputFocus(newWindow); 

    // Create OpenGL context if it's the first window
    if (count == 0) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        glContext = SDL_GL_CreateContext(newWindow);
        if (!glContext) {
            std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << "\n";
            SDL_DestroyWindow(newWindow);
            exit(EXIT_FAILURE);
        }

        // Initialize GLEW after creating the OpenGL context
        glewExperimental = GL_TRUE;  // Enable experimental extensions
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            std::cerr << "GLEW initialization failed: " << glewGetErrorString(err) << "\n";
            SDL_DestroyWindow(newWindow);
            exit(EXIT_FAILURE);
        }

        // Ensure OpenGL is working
        GLenum glErr;
        while ((glErr = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL Error: " << glErr << "\n";
        }
    }

    // Add window to list
    windows[count] = newWindow;
    ++count;

    std::cout << "Window created successfully: " << title << "\n";
}

void SDL_Manager::closeWindow(uint32_t id) {
    for (size_t i = 0; i < count; ++i) {
        if (SDL_GetWindowID(windows[i]) == id) {
            SDL_DestroyWindow(windows[i]);

            // If the OpenGL context window is closed, shutdown everything
            if (i == 0 && glContext) {
                SDL_GL_DeleteContext(glContext);
                glContext = nullptr;
                exit(0);
            }

            // Swap with the last window in the list and remove
            windows[i] = windows[count - 1];
            windows[count - 1] = nullptr;

            --count;

            std::cout << "Window with ID " << id << " closed successfully.\n";
            return;
        }
    }
    std::cerr << "Window with ID " << id << " not found.\n";
}

void SDL_Manager::updateWindows() {
    if (count > 0) {
        SDL_GL_SwapWindow(windows[0]); // Swaps buffers properly
    }

    for (size_t i = 1; i < count; ++i) {
        SDL_UpdateWindowSurface(windows[i]); // Updates non-OpenGL windows
    }
}