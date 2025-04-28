#include "Breakout.hpp"
#include <algorithm>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

// Shape creation helper functions
Shape createQuadShape() {
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

Shape createCircleShape(int segments) {
    std::vector<float> vertexData;
    float centerX = 0.0f;
    float centerY = 0.0f;
    float centerZ = 0.0f;
    
    // Reserve space for all vertices (positions and normals)
    vertexData.reserve(segments * 6 * 3); // 6 floats per vertex (pos + normal), 3 vertices per triangle
    
    for (int i = 0; i < segments; i++) {
        float angle1 = 2.0f * 3.14159f * i / segments;
        float angle2 = 2.0f * 3.14159f * (i + 1) / segments;
        
        float x1 = 0.5f * cos(angle1);
        float y1 = 0.5f * sin(angle1);
        
        float x2 = 0.5f * cos(angle2);
        float y2 = 0.5f * sin(angle2);
        
        // Add positions for all vertices first
        // First vertex (center)
        vertexData.push_back(centerX);
        vertexData.push_back(centerY);
        vertexData.push_back(centerZ);
        
        // Second vertex
        vertexData.push_back(x1);
        vertexData.push_back(y1);
        vertexData.push_back(centerZ);
        
        // Third vertex
        vertexData.push_back(x2);
        vertexData.push_back(y2);
        vertexData.push_back(centerZ);
    }
    
    // Then add normals for all vertices
    for (int i = 0; i < segments * 3; i++) {
        // Same normal for all vertices (facing out of screen)
        vertexData.push_back(0.0f);
        vertexData.push_back(0.0f);
        vertexData.push_back(1.0f);
    }
    
    return Shape(segments, vertexData);
}

// Paddle implementation
Paddle::Paddle(const glm::vec3& pos, float width, float height, const Shape& shape, int id)
    : GameObject(pos, Quaternion(0.0f, glm::vec3(0.0f, 0.0f, 1.0f)), shape, id),
      width(width), height(height), speed(10.0f) {
    
    // Scale the model matrix to the paddle size
    modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
                  glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
}

void Paddle::update(float deltaTime) {
    // No automatic movement
}

void Paddle::moveLeft(float dt) {
    position.x -= speed * dt;
    
    // Update model matrix
    modelMatrix = glm::translate(glm::mat4(1.0f), position) *
                  glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
}

void Paddle::moveRight(float dt) {
    position.x += speed * dt;
    
    // Update model matrix
    modelMatrix = glm::translate(glm::mat4(1.0f), position) *
                  glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
}

bool Paddle::intersects(const Ball& ball) const {
    // AABB collision detection with ball
    float paddleLeft = position.x - width / 2.0f;
    float paddleRight = position.x + width / 2.0f;
    float paddleTop = position.y + height / 2.0f;
    float paddleBottom = position.y - height / 2.0f;
    
    glm::vec3 ballPos = ball.getModelMatrix()[3];
    float ballRadius = ball.getRadius();
    
    // Check if ball intersects with paddle
    return (ballPos.x + ballRadius >= paddleLeft &&
            ballPos.x - ballRadius <= paddleRight &&
            ballPos.y + ballRadius >= paddleBottom &&
            ballPos.y - ballRadius <= paddleTop);
}

// Ball implementation
Ball::Ball(const glm::vec3& pos, float radius, const Shape& shape, int id)
    : GameObject(pos, Quaternion(0.0f, glm::vec3(0.0f, 0.0f, 1.0f)), shape, id),
      radius(radius), velocity(0.0f, 0.0f, 0.0f), stuck(true) {
    
    // Scale the model matrix to the ball size
    modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
                  glm::scale(glm::mat4(1.0f), glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));
}

void Ball::update(float deltaTime) {
    if (!stuck) {
        // Move ball according to its velocity
        position += velocity * deltaTime;
        
        // Update model matrix
        modelMatrix = glm::translate(glm::mat4(1.0f), position) *
                      glm::scale(glm::mat4(1.0f), glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));
    }
}

void Ball::launch() {
    if (stuck) {
        stuck = false;
        // Launch at a 45-degree angle upward
        velocity = glm::vec3(5.0f, 5.0f, 0.0f);
    }
}

void Ball::stickToPaddle(const Paddle& paddle) {
    // Position the ball on top of the paddle
    position.x = paddle.getModelMatrix()[3].x;
    position.y = paddle.getModelMatrix()[3].y + paddle.getHeight() / 2.0f + radius;
    
    // Update model matrix
    modelMatrix = glm::translate(glm::mat4(1.0f), position) *
                  glm::scale(glm::mat4(1.0f), glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));
}

// Brick implementation
Brick::Brick(const glm::vec3& pos, float width, float height, int hitPoints, int scoreValue, const Shape& shape, int id)
    : GameObject(pos, Quaternion(0.0f, glm::vec3(0.0f, 0.0f, 1.0f)), shape, id),
      width(width), height(height), destroyed(false), hitPoints(hitPoints), scoreValue(scoreValue) {
    
    // Scale the model matrix to the brick size
    modelMatrix = glm::translate(glm::mat4(1.0f), pos) *
                  glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
}

void Brick::update(float deltaTime) {
    // No movement or animation for bricks
}

bool Brick::intersects(const Ball& ball) const {
    if (destroyed) {
        return false;
    }
    
    // AABB collision detection with ball
    float brickLeft = position.x - width / 2.0f;
    float brickRight = position.x + width / 2.0f;
    float brickTop = position.y + height / 2.0f;
    float brickBottom = position.y - height / 2.0f;
    
    glm::vec3 ballPos = ball.getModelMatrix()[3];
    float ballRadius = ball.getRadius();
    
    // Check if ball intersects with brick
    return (ballPos.x + ballRadius >= brickLeft &&
            ballPos.x - ballRadius <= brickRight &&
            ballPos.y + ballRadius >= brickBottom &&
            ballPos.y - ballRadius <= brickTop);
}

void Brick::hit() {
    hitPoints--;
    if (hitPoints <= 0) {
        destroyed = true;
    }
}

// Breakout game implementation
Breakout::Breakout(SoundSystem& soundSystem, float left, float right, float top, float bottom)
    : state(GameState::Start),
      score(0),
      lives(3),
      soundSystem(soundSystem),
      leftBoundary(left),
      rightBoundary(right),
      topBoundary(top),
      bottomBoundary(bottom) {
    
    // Create scene graph with boundaries as world bounds
    AABB worldBounds(glm::vec3(left, bottom, -10.0f), glm::vec3(right, top, 10.0f));
    sceneGraph = std::make_unique<SceneGraph>(worldBounds);
    
    // Load game sounds
    paddleHitSound = soundSystem.loadSound("../audio/paddle_hit.wav");
    brickHitSound = soundSystem.loadSound("../audio/brick_hit.wav");
    wallHitSound = soundSystem.loadSound("../audio/wall_hit.wav");
    loseLifeSound = soundSystem.loadSound("../audio/lose_life.wav");
    
    std::cout << "Creating shapes directly..." << std::endl;
    
    // Create shapes as local variables
    Shape paddleShape = createQuadShape();
    std::cout << "Paddle shape created." << std::endl;
    
    Shape ballShape = createCircleShape(16);
    std::cout << "Ball shape created." << std::endl;
    
    Shape brickShape = createQuadShape();
    std::cout << "Brick shape created." << std::endl;
    
    // Store copies in member vector for persistence
    shapes.reserve(3); // Pre-allocate to avoid reallocation
    shapes.push_back(paddleShape);
    shapes.push_back(ballShape);
    shapes.push_back(brickShape);
    
    std::cout << "Shapes copied to storage. Creating game objects..." << std::endl;
    
    // Create paddle
    float paddleWidth = 0.15f * (right - left);
    float paddleHeight = 0.03f * (top - bottom);
    paddle = std::make_unique<Paddle>(
        glm::vec3((left + right) / 2.0f, bottom + paddleHeight, 0.0f),
        paddleWidth, paddleHeight, shapes[0], 1);
    
    // Create ball
    float ballRadius = 0.02f * (right - left);
    ball = std::make_unique<Ball>(
        glm::vec3((left + right) / 2.0f, bottom + paddleHeight * 2.0f + ballRadius, 0.0f),
        ballRadius, shapes[1], 2);
    
    // Create bricks
    int rows = 5;
    int cols = 8;
    float brickWidth = (right - left) / (cols + 2);  // Leave space on sides
    float brickHeight = 0.04f * (top - bottom);
    float spacing = 0.01f * (right - left);
    createBricks(rows, cols, brickWidth, brickHeight, spacing);
    
    // Add objects to scene graph
    sceneGraph->addObject(paddle.get());
    sceneGraph->addObject(ball.get());
    
    std::cout << "Adding " << bricks.size() << " bricks to scene graph" << std::endl;
    for (auto& brick : bricks) {
        sceneGraph->addObject(brick.get());
    }
    
    std::cout << "Game initialization complete" << std::endl;
    std::cout << "Paddle created at: " << paddle->getModelMatrix()[3].x << ", " 
              << paddle->getModelMatrix()[3].y << ", " 
              << paddle->getModelMatrix()[3].z << std::endl;
    std::cout << "Paddle dimensions: " << paddle->getWidth() << " x " << paddle->getHeight() << std::endl;
    std::cout << "Ball radius: " << ball->getRadius() << std::endl;
}

Breakout::~Breakout() {
    // Clean up resources if needed
    std::cout << "Breakout game destroyed" << std::endl;
}

void Breakout::createBricks(int rows, int cols, float width, float height, float spacing) {
    // Create brick layout
    // Use the stored brick shape (index 2)
    const Shape& brickShape = shapes[2];
    
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            // Calculate brick position
            float x = leftBoundary + (col + 1.5f) * width;
            float y = topBoundary - (row + 1.5f) * (height + spacing);
            
            // Determine hit points and score value based on row
            int hitPoints = rows - row;
            int scoreValue = (rows - row) * 10;
            
            // Create brick
            auto brick = std::make_unique<Brick>(
                glm::vec3(x, y, 0.0f),
                width - spacing, height - spacing,
                hitPoints, scoreValue, brickShape, 3 + row * cols + col);
            
            bricks.push_back(std::move(brick));
        }
    }
    
    std::cout << "Created " << bricks.size() << " bricks" << std::endl;
}

void Breakout::resetBall() {
    ball->setStuck(true);
    ball->stickToPaddle(*paddle);
}

void Breakout::update(float deltaTime) {
    // Skip updates if game is not playing
    if (state != GameState::Playing && state != GameState::Start) {
        return;
    }
    
    // Update game objects
    paddle->update(deltaTime);
    ball->update(deltaTime);
    
    for (auto& brick : bricks) {
        brick->update(deltaTime);
    }
    
    // Handle ball-paddle collision
    if (paddle->intersects(*ball)) {
        // Only bounce if moving downward
        if (ball->getVelocity().y < 0.0f) {
            // Play sound
            soundSystem.playSound(paddleHitSound);
            
            // Calculate bounce angle based on where the ball hit the paddle
            float paddleCenter = paddle->getModelMatrix()[3].x;
            float ballCenter = ball->getModelMatrix()[3].x;
            float ratio = (ballCenter - paddleCenter) / (paddle->getWidth() / 2.0f);
            
            // Limit ratio to prevent too horizontal bounces
            ratio = std::clamp(ratio, -0.8f, 0.8f);
            
            // Calculate new velocity
            float speed = glm::length(ball->getVelocity());
            glm::vec3 newVelocity(speed * ratio, std::abs(ball->getVelocity().y), 0.0f);
            
            // Normalize and scale
            newVelocity = glm::normalize(newVelocity) * speed;
            
            // Ensure upward direction
            if (newVelocity.y < 0.5f) {
                newVelocity.y = 0.5f;
                newVelocity = glm::normalize(newVelocity) * speed;
            }
            
            ball->setVelocity(newVelocity);
        }
    }
    
    // Handle ball-brick collisions
    for (auto& brick : bricks) {
        if (!brick->isDestroyed() && brick->intersects(*ball)) {
            // Play sound
            soundSystem.playSound(brickHitSound);
            
            // Damage brick
            brick->hit();
            
            // Add score if destroyed
            if (brick->isDestroyed()) {
                score += brick->getScoreValue();
            }
            
            // Calculate collision response
            glm::vec3 ballPos = ball->getModelMatrix()[3];
            glm::vec3 brickPos = brick->getModelMatrix()[3];
            float brickWidth = brick->getWidth();
            float brickHeight = brick->getHeight();
            
            // Determine collision side
            float overlapLeft = (brickPos.x - brickWidth / 2.0f) - (ballPos.x + ball->getRadius());
            float overlapRight = (ballPos.x - ball->getRadius()) - (brickPos.x + brickWidth / 2.0f);
            float overlapTop = (brickPos.y - brickHeight / 2.0f) - (ballPos.y + ball->getRadius());
            float overlapBottom = (ballPos.y - ball->getRadius()) - (brickPos.y + brickHeight / 2.0f);
            
            // Find smallest overlap to determine collision side
            float minOverlap = std::min({
                std::abs(overlapLeft), 
                std::abs(overlapRight),
                std::abs(overlapTop),
                std::abs(overlapBottom)
            });
            
            // Respond based on collision side
            if (minOverlap == std::abs(overlapLeft) || minOverlap == std::abs(overlapRight)) {
                ball->reverseX();
            } else {
                ball->reverseY();
            }
            
            // Only process one collision per frame
            break;
        }
    }
    
    // Handle wall collisions
    glm::vec3 ballPos = ball->getModelMatrix()[3];
    float ballRadius = ball->getRadius();
    
    // Left and right walls
    if (ballPos.x - ballRadius <= leftBoundary) {
        soundSystem.playSound(wallHitSound);
        ball->setVelocity(glm::vec3(std::abs(ball->getVelocity().x), ball->getVelocity().y, 0.0f));
    } else if (ballPos.x + ballRadius >= rightBoundary) {
        soundSystem.playSound(wallHitSound);
        ball->setVelocity(glm::vec3(-std::abs(ball->getVelocity().x), ball->getVelocity().y, 0.0f));
    }
    
    // Top wall
    if (ballPos.y + ballRadius >= topBoundary) {
        soundSystem.playSound(wallHitSound);
        ball->setVelocity(glm::vec3(ball->getVelocity().x, -std::abs(ball->getVelocity().y), 0.0f));
    }
    
    // Bottom wall - lose life
    if (ballPos.y - ballRadius <= bottomBoundary) {
        soundSystem.playSound(loseLifeSound);
        lives--;
        
        if (lives <= 0) {
            state = GameState::GameOver;
        } else {
            resetBall();
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
        state = GameState::Win;
    }
    
    // Update scene graph
    sceneGraph->updateTransforms();
    sceneGraph->updateSpatialStructure();
}

void Breakout::handleInput(float dt, bool leftPressed, bool rightPressed, bool spacePressed) {
    // Start game with space
    if (state == GameState::Start && spacePressed) {
        state = GameState::Playing;
        ball->launch();
    } else if (state == GameState::Playing) {
        // Move paddle left/right
        if (leftPressed) {
            // Ensure paddle stays within bounds
            float newX = paddle->getModelMatrix()[3].x - paddle->getSpeed() * dt;
            if (newX - paddle->getWidth() / 2.0f > leftBoundary) {
                paddle->moveLeft(dt);
                if (ball->isStuck()) {
                    ball->stickToPaddle(*paddle);
                }
            }
        }
        
        if (rightPressed) {
            // Ensure paddle stays within bounds
            float newX = paddle->getModelMatrix()[3].x + paddle->getSpeed() * dt;
            if (newX + paddle->getWidth() / 2.0f < rightBoundary) {
                paddle->moveRight(dt);
                if (ball->isStuck()) {
                    ball->stickToPaddle(*paddle);
                }
            }
        }
        
        // Launch ball with space
        if (spacePressed && ball->isStuck()) {
            ball->launch();
        }
    } else if ((state == GameState::GameOver || state == GameState::Win) && spacePressed) {
        // Restart game
        reset();
    }
}

void Breakout::reset() {
    // Reset game state
    state = GameState::Start;
    score = 0;
    lives = 3;
    
    // Reset paddle position
    paddle->setPosition(glm::vec3((leftBoundary + rightBoundary) / 2.0f, 
                                 bottomBoundary + paddle->getHeight(), 0.0f));
    
    // Reset ball
    resetBall();
    
    // Reset bricks
    bricks.clear();
    int rows = 5;
    int cols = 8;
    float brickWidth = (rightBoundary - leftBoundary) / (cols + 2);
    float brickHeight = 0.04f * (topBoundary - bottomBoundary);
    float spacing = 0.01f * (rightBoundary - leftBoundary);
    createBricks(rows, cols, brickWidth, brickHeight, spacing);
    
    // Update scene graph
    sceneGraph = std::make_unique<SceneGraph>(AABB(
        glm::vec3(leftBoundary, bottomBoundary, -10.0f), 
        glm::vec3(rightBoundary, topBoundary, 10.0f)));
    
    sceneGraph->addObject(paddle.get());
    sceneGraph->addObject(ball.get());
    for (auto& brick : bricks) {
        sceneGraph->addObject(brick.get());
    }
}