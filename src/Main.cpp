#include "SDL_Manager.hpp"
#include "Shader.hpp"
#include "Engine.hpp"
#include "Renderer.hpp"
#include "GameObject.hpp"
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
    sdl.spawnWindow("Breakout Game", 800, 600, SDL_TRUE);

    if (!initOpenGL()) return EXIT_FAILURE;
    
    // Initialize sound system
    SoundSystem soundSystem;

    // Set background color to very dark blue for better contrast
    glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
    
    // Load shader
    Shader shader("../shader.vert", "../shader.frag");
    Renderer renderer;
    
    // Create camera for 2D view
    Camera camera(60.0f, 800.0f / 600.0f, 0.1f, 100.0f);
    camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Game boundaries
    float gameWidth = 10.0f;
    float gameHeight = 14.0f;
    float leftBoundary = -gameWidth / 2.0f;
    float rightBoundary = gameWidth / 2.0f;
    float topBoundary = gameHeight / 2.0f;
    float bottomBoundary = -gameHeight / 2.0f;
    
    // Create shapes only once
    Shape quadShape = createQuadShape();
    Shape circleShape = createCircleShape(32);
    
    // Create paddle - making it MUCH larger and more visible
    float paddleWidth = 3.0f;
    float paddleHeight = 0.9f;
    float paddleY = bottomBoundary + 1.0f;
    
    Paddle paddle(
        glm::vec3(0.0f, paddleY, 0.0f),
        paddleWidth, paddleHeight,
        quadShape, 1
    );
    
    // Create ball - making it larger for visibility
    float ballRadius = 0.6f;
    Ball ball(
        glm::vec3(0.0f, paddleY + paddleHeight + ballRadius, 0.0f),
        ballRadius,
        circleShape, 2
    );
    
    // Create bricks
    int rows = 3;
    int cols = 5;
    float brickWidth = 1.5f;
    float brickHeight = 0.5f;
    float spacing = 0.2f;
    float startY = topBoundary - 2.0f;
    
    std::vector<std::unique_ptr<Brick>> bricks;
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            float x = leftBoundary + (col + 1) * (brickWidth + spacing);
            float y = startY - row * (brickHeight + spacing);
            
            // Different hit points based on row
            int hitPoints = 3 - row;  // Top row has 3 hit points, bottom row has 1
            if (hitPoints < 1) hitPoints = 1;
            int scoreValue = (row + 1) * 10;  // Higher rows are worth more points
            
            auto brick = std::make_unique<Brick>(
                glm::vec3(x, y, 0.0f),
                brickWidth, brickHeight,
                hitPoints, scoreValue, quadShape, 3 + row * cols + col
            );
            
            bricks.push_back(std::move(brick));
        }
    }
    
    // Game state
    bool ballLaunched = false;
    int score = 0;
    int lives = 3;
    enum class GameState { Start, Playing, GameOver, Win } gameState = GameState::Start;
    
    // Load sounds
    int paddleHitSound = soundSystem.loadSound("../audioa/paddle_hit.wav");
    int brickHitSound = soundSystem.loadSound("../audio/brick_hit.wav");
    int wallHitSound = soundSystem.loadSound("../audioa/wall_hit.wav");
    int loseLifeSound = soundSystem.loadSound("../audio/lose_life.wav");
    
    // If sound files don't exist, output a message
    std::cout << "Note: If sound files are missing, the game will still run but without audio." << std::endl;

    // Main loop
    bool exit = false;
    SDL_Event e;
    
    // Key state tracking
    bool leftPressed = false;
    bool rightPressed = false;
    
    // For resetting the game - we need to store these to recreate the bricks
    Shape* brickShapePtr = &quadShape;

    while (!exit) {
        Engine::update();
        float dt = Engine::getDeltaSeconds();
        
        // Process events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit = true;
            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
                sdl.closeWindow(e.window.windowID);
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        exit = true;
                        break;
                    case SDLK_LEFT:
                        leftPressed = true;
                        break;
                    case SDLK_RIGHT:
                        rightPressed = true;
                        break;
                    case SDLK_SPACE:
                        // Start game if in start state
                        if (gameState == GameState::Start) {
                            gameState = GameState::Playing;
                            ballLaunched = true;
                            ball.launch();
                        }
                        // Launch ball if it's stuck to paddle
                        else if (gameState == GameState::Playing && ball.isStuck()) {
                            ball.launch();
                        }
                        // Restart game if game over or win
                        else if (gameState == GameState::GameOver || gameState == GameState::Win) {
                            // Reset game state
                            gameState = GameState::Start;
                            score = 0;
                            lives = 3;
                            
                            // Reset paddle position
                            paddle.setPosition(glm::vec3(0.0f, paddleY, 0.0f));
                            
                            // Reset ball
                            ball.setStuck(true);
                            ball.stickToPaddle(paddle);
                            
                            // Reset bricks
                            bricks.clear();
                            for (int row = 0; row < rows; row++) {
                                for (int col = 0; col < cols; col++) {
                                    float x = leftBoundary + (col + 1) * (brickWidth + spacing);
                                    float y = startY - row * (brickHeight + spacing);
                                    
                                    int hitPoints = 3 - row;
                                    if (hitPoints < 1) hitPoints = 1;
                                    int scoreValue = (row + 1) * 10;
                                    
                                    auto brick = std::make_unique<Brick>(
                                        glm::vec3(x, y, 0.0f),
                                        brickWidth, brickHeight,
                                        hitPoints, scoreValue, *brickShapePtr, 3 + row * cols + col
                                    );
                                    
                                    bricks.push_back(std::move(brick));
                                }
                            }
                        }
                        break;
                }
            }
            else if (e.type == SDL_KEYUP) {
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

        // Game logic (only if playing)
        if (gameState == GameState::Playing) {
            // Move paddle based on input
            if (leftPressed) {
                float newX = paddle.getModelMatrix()[3].x - paddle.getSpeed() * dt;
                if (newX - paddleWidth / 2.0f > leftBoundary) {
                    paddle.moveLeft(dt);
                    if (ball.isStuck()) {
                        ball.stickToPaddle(paddle);
                    }
                }
            }
            
            if (rightPressed) {
                float newX = paddle.getModelMatrix()[3].x + paddle.getSpeed() * dt;
                if (newX + paddleWidth / 2.0f < rightBoundary) {
                    paddle.moveRight(dt);
                    if (ball.isStuck()) {
                        ball.stickToPaddle(paddle);
                    }
                }
            }
            
            // Update ball if launched
            ball.update(dt);
            
            // Handle ball-paddle collision
            if (paddle.intersects(ball)) {
                if (ball.getVelocity().y < 0.0f) {  // Only bounce if moving downward
                    // Play sound
                    // soundSystem.playSound(paddleHitSound);
                    
                    // Calculate bounce angle based on where ball hit paddle
                    float paddleCenter = paddle.getModelMatrix()[3].x;
                    float ballCenter = ball.getModelMatrix()[3].x;
                    float ratio = (ballCenter - paddleCenter) / (paddle.getWidth() / 2.0f);
                    
                    // Limit ratio to prevent too horizontal bounces
                    ratio = std::clamp(ratio, -0.8f, 0.8f);
                    
                    // Calculate new velocity
                    float speed = glm::length(ball.getVelocity());
                    glm::vec3 newVelocity(speed * ratio, std::abs(ball.getVelocity().y), 0.0f);
                    
                    // Normalize and scale
                    newVelocity = glm::normalize(newVelocity) * speed;
                    
                    // Ensure upward direction
                    if (newVelocity.y < 0.5f) {
                        newVelocity.y = 0.5f;
                        newVelocity = glm::normalize(newVelocity) * speed;
                    }
                    
                    ball.setVelocity(newVelocity);
                }
            }
            
            // Ball-wall collisions
            glm::vec3 ballPos = ball.getModelMatrix()[3];
            float ballR = ball.getRadius();
            
            // Left and right walls
            if (ballPos.x - ballR <= leftBoundary) {
                //soundSystem.playSound(wallHitSound);
                ball.setVelocity(glm::vec3(std::abs(ball.getVelocity().x), ball.getVelocity().y, 0.0f));
            } else if (ballPos.x + ballR >= rightBoundary) {
                //soundSystem.playSound(wallHitSound);
                ball.setVelocity(glm::vec3(-std::abs(ball.getVelocity().x), ball.getVelocity().y, 0.0f));
            }
            
            // Top wall
            if (ballPos.y + ballR >= topBoundary) {
                //soundSystem.playSound(wallHitSound);
                ball.setVelocity(glm::vec3(ball.getVelocity().x, -std::abs(ball.getVelocity().y), 0.0f));
            }
            
            // Bottom wall - lose life
            if (ballPos.y - ballR <= bottomBoundary) {
                soundSystem.playSound(loseLifeSound);
                lives--;
                
                if (lives <= 0) {
                    gameState = GameState::GameOver;
                } else {
                    // Reset ball
                    ball.setStuck(true);
                    ball.stickToPaddle(paddle);
                }
            }
            
            // Ball-brick collisions
            for (auto& brick : bricks) {
                if (!brick->isDestroyed() && brick->intersects(ball)) {
                    // Play sound
                    soundSystem.playSound(brickHitSound);
                    
                    // Damage brick
                    brick->hit();
                    
                    // Add score if destroyed
                    if (brick->isDestroyed()) {
                        score += brick->getScoreValue();
                    }
                    
                    // Calculate collision response
                    glm::vec3 ballPos = ball.getModelMatrix()[3];
                    glm::vec3 brickPos = brick->getModelMatrix()[3];
                    float brickWidth = brick->getWidth();
                    float brickHeight = brick->getHeight();
                    
                    // Determine collision side
                    float overlapLeft = (brickPos.x - brickWidth / 2.0f) - (ballPos.x + ball.getRadius());
                    float overlapRight = (ballPos.x - ball.getRadius()) - (brickPos.x + brickWidth / 2.0f);
                    float overlapTop = (brickPos.y - brickHeight / 2.0f) - (ballPos.y + ball.getRadius());
                    float overlapBottom = (ballPos.y - ball.getRadius()) - (brickPos.y + brickHeight / 2.0f);
                    
                    // Find smallest overlap to determine collision side
                    float minOverlap = std::min({
                        std::abs(overlapLeft), 
                        std::abs(overlapRight),
                        std::abs(overlapTop),
                        std::abs(overlapBottom)
                    });
                    
                    // Respond based on collision side
                    if (minOverlap == std::abs(overlapLeft) || minOverlap == std::abs(overlapRight)) {
                        ball.reverseX();
                    } else {
                        ball.reverseY();
                    }
                    
                    // Only process one collision per frame
                    break;
                }
            }
            
            // Check win condition
            bool allBricksDestroyed = true;
            for (const auto& brick : bricks) {
                if (!brick->isDestroyed()) {
                    allBricksDestroyed = false;
                    break;
                }
            }
            
            if (allBricksDestroyed) {
                gameState = GameState::Win;
            }
        }

        // Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Disable depth test to ensure all objects are visible
        glDisable(GL_DEPTH_TEST);
        
        // Submit objects to renderer
        renderer.submit(&paddle);
        renderer.submit(&ball);
        
        for (const auto& brick : bricks) {
            if (!brick->isDestroyed()) {
                renderer.submit(brick.get());
            }
        }
        
        // Re-enable depth test
        glEnable(GL_DEPTH_TEST);
        
        // Render using your renderer
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix();
        renderer.render(shader, view, proj);
        
        // Update window
        sdl.updateWindows();
        std::this_thread::sleep_for(16ms); // Fixed ~60 FPS
        
        // Print game state (for debugging)
        static float printTimer = 0.0f;
        printTimer += dt;
        if (printTimer >= 1.0f) {
            std::cout << "Lives: " << lives << " Score: " << score;
            switch (gameState) {
                case GameState::Start: std::cout << " State: Start"; break;
                case GameState::Playing: std::cout << " State: Playing"; break;
                case GameState::GameOver: std::cout << " State: Game Over"; break;
                case GameState::Win: std::cout << " State: Win"; break;
            }
            std::cout << std::endl;
            printTimer = 0.0f;
        }
    }

    return 0;
}