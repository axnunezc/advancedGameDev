#ifndef BREAKOUT_HPP
#define BREAKOUT_HPP

#include "GameObject.hpp"
#include "SoundSystem.hpp"
#include "SceneGraph.hpp"
#include <vector>
#include <memory>

// Forward declarations
class Paddle;
class Ball;
class Brick;

// Game state enum
enum class GameState {
    Start,
    Playing,
    GameOver,
    Win
};

// Helper functions for shape creation
Shape createQuadShape();
Shape createCircleShape(int segments = 16);

// Breakout game class
class Breakout {
private:
    // Game objects
    std::unique_ptr<Paddle> paddle;
    std::unique_ptr<Ball> ball;
    std::vector<std::unique_ptr<Brick>> bricks;
    
    // Store shapes to prevent them from being destroyed
    std::vector<Shape> shapes;
    
    // Game state
    GameState state;
    int score;
    int lives;
    
    // Scene graph for spatial management
    std::unique_ptr<SceneGraph> sceneGraph;
    
    // Sound system reference
    SoundSystem& soundSystem;
    
    // Sound indices
    int paddleHitSound;
    int brickHitSound;
    int wallHitSound;
    int loseLifeSound;
    
    // Game boundaries
    float leftBoundary;
    float rightBoundary;
    float topBoundary;
    float bottomBoundary;
    
    // Create game objects
    void createBricks(int rows, int cols, float width, float height, float spacing);
    void resetBall();
    
public:
    Breakout(SoundSystem& soundSystem, float left, float right, float top, float bottom);
    ~Breakout();
    
    // Game logic
    void update(float deltaTime);
    void handleInput(float dt, bool leftPressed, bool rightPressed, bool spacePressed);
    
    // Scene access for rendering
    SceneGraph* getSceneGraph() const { return sceneGraph.get(); }
    
    // Game state access
    GameState getState() const { return state; }
    int getScore() const { return score; }
    int getLives() const { return lives; }
    
    // Reset game
    void reset();
};

// Paddle game object
class Paddle : public GameObject {
private:
    float width;
    float height;
    float speed;
    
public:
    Paddle(const glm::vec3& pos, float width, float height, const Shape& shape, int id);
    
    void update(float deltaTime);
    void moveLeft(float dt);
    void moveRight(float dt);
    
    // Collision detection
    bool intersects(const Ball& ball) const;
    
    // Getters
    float getWidth() const { return width; }
    float getHeight() const { return height; }
    float getSpeed() const { return speed; }
    
    // For reset
    void setPosition(const glm::vec3& pos) { 
        position = pos;
        modelMatrix = glm::translate(glm::mat4(1.0f), pos) * 
                      glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f)); 
    }
};

// Ball game object
class Ball : public GameObject {
private:
    float radius;
    glm::vec3 velocity;
    bool stuck;  // If true, ball follows paddle before launch
    
public:
    Ball(const glm::vec3& pos, float radius, const Shape& shape, int id);
    
    void update(float deltaTime);
    
    // Ball control
    void launch();
    void setStuck(bool stuck) { this->stuck = stuck; }
    bool isStuck() const { return stuck; }
    
    // Position the ball relative to the paddle
    void stickToPaddle(const Paddle& paddle);
    
    // Physics
    void setVelocity(const glm::vec3& vel) { velocity = vel; }
    glm::vec3 getVelocity() const { return velocity; }
    
    // Collision response
    void reverseX() { velocity.x = -velocity.x; }
    void reverseY() { velocity.y = -velocity.y; }
    
    // Getters
    float getRadius() const { return radius; }
};

// Brick game object
class Brick : public GameObject {
private:
    float width;
    float height;
    bool destroyed;
    int hitPoints;
    int scoreValue;
    
public:
    Brick(const glm::vec3& pos, float width, float height, int hitPoints, int scoreValue, const Shape& shape, int id);
    
    void update(float deltaTime);
    
    // Collision detection
    bool intersects(const Ball& ball) const;
    
    // Brick state
    bool isDestroyed() const { return destroyed; }
    void hit();
    
    // Getters
    float getWidth() const { return width; }
    float getHeight() const { return height; }
    int getScoreValue() const { return scoreValue; }
};

#endif // BREAKOUT_HPP