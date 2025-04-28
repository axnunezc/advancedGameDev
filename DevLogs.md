# Breakout Game Implementation Documentation

This document provides an overview of the Breakout game implementation, including detailed explanations of each class and file in the project.

## Table of Contents

1. [Project Overview](#project-overview)
2. [Game Architecture](#game-architecture)
3. [Core Files](#core-files)
4. [Game Objects](#game-objects)
5. [Rendering](#rendering)
6. [Audio](#audio)
7. [Scene Management](#scene-management)
8. [Game Flow](#game-flow)

## Project Overview

This project includes an implementation of the classic Breakout game using C++ with SDL for window management and input handling, and OpenGL for rendering. The game features a paddle controlled by the player, a ball that bounces around the screen, and bricks that must be destroyed to win.

### Features

- 2D rendering with OpenGL
- Physics-based ball movement and collisions
- Multiple brick types with different hit points
- Score tracking
- Lives system
- Sound effects
- Win/lose conditions

## Game Architecture

The game is structured around several key components:

- **SDL Manager**: Handles window creation and management
- **Engine**: Manages game timing and updates
- **Renderer**: Handles rendering of game objects
- **SceneGraph**: Manages spatial relationships between objects
- **Shape**: Represents mesh data for game objects
- **GameObject**: Base class for all game entities
- **SoundSystem**: Handles audio playback
- **Breakout**: Main game class that coordinates gameplay

## Core Files

### SDL_Manager.hpp/cpp

The SDL Manager is responsible for initializing SDL, creating and managing windows, and handling OpenGL context.

```cpp
class SDL_Manager {
private:
    static const int MAX_WINDOWS = 10;
    SDL_Window* windows[MAX_WINDOWS];
    SDL_GLContext glContext;
    int count;

    SDL_Manager();
    ~SDL_Manager();

public:
    static SDL_Manager& sdl();
    void spawnWindow(const std::string& title, int width, int height, SDL_bool resizable);
    void closeWindow(uint32_t id);
    void updateWindows();
};
```

Key functionalities:
- Singleton pattern for global access
- Window creation and management
- OpenGL context setup
- Window updates and cleanup

### Engine.hpp/cpp

The Engine class handles game timing and main loop updates.

```cpp
namespace Engine {
    void initialize();
    void update();
    std::uint32_t getDeltaTime();
    float getDeltaSeconds();
}
```

Key functionalities:
- Game initialization
- Delta time calculation for frame-rate independent updates
- Time-based updates

### Utility.hpp/cpp

Contains utility functions and variables used throughout the project.

```cpp
namespace Utility {
    extern std::chrono::steady_clock::time_point prevTime;
    extern std::uint32_t deltaTime;
    extern float deltaSeconds;
    
    void updateDeltaTime();
}
```

Key functionalities:
- Time tracking
- Delta time calculation

## Game Objects

### GameObject.hpp/cpp

The base class for all game entities.

```cpp
class GameObject {
protected:
    glm::vec3 position;
    Quaternion rotation;
    glm::mat4 modelMatrix;
    const Shape& renderElementShape;
    int renderElement;
    
public:
    GameObject(glm::vec3 pos, Quaternion rot, const Shape& shape, int id);
    virtual ~GameObject() = default;
    
    virtual void update(float deltaTime) = 0;
    
    const glm::mat4& getModelMatrix() const;
    GLuint getVAO() const;
    GLuint getVBO() const;
    int getVertexCount() const;
};
```

Key functionalities:
- Position and rotation management
- Model matrix calculation
- Shape and rendering element references
- Virtual update method for derived classes

### Breakout.hpp/cpp

The main game class that implements the Breakout game logic.

```cpp
class Breakout {
private:
    std::unique_ptr<Paddle> paddle;
    std::unique_ptr<Ball> ball;
    std::vector<std::unique_ptr<Brick>> bricks;
    std::vector<Shape> shapes;
    
    GameState state;
    int score;
    int lives;
    
    std::unique_ptr<SceneGraph> sceneGraph;
    SoundSystem& soundSystem;
    
    int paddleHitSound;
    int brickHitSound;
    int wallHitSound;
    int loseLifeSound;
    
    float leftBoundary;
    float rightBoundary;
    float topBoundary;
    float bottomBoundary;
    
    void createBricks(int rows, int cols, float width, float height, float spacing);
    void resetBall();
    
public:
    Breakout(SoundSystem& soundSystem, float left, float right, float top, float bottom);
    ~Breakout();
    
    void update(float deltaTime);
    void handleInput(float dt, bool leftPressed, bool rightPressed, bool spacePressed);
    
    SceneGraph* getSceneGraph() const;
    GameState getState() const;
    int getScore() const;
    int getLives() const;
    
    void reset();
};
```

Key functionalities:
- Game initialization and setup
- Game loop management
- Input handling
- Collision detection and response
- Game state management
- Object creation and management

### Paddle.hpp/cpp (Part of Breakout.hpp)

Represents the player-controlled paddle.

```cpp
class Paddle : public GameObject {
private:
    float width;
    float height;
    float speed;
    
public:
    Paddle(const glm::vec3& pos, float width, float height, const Shape& shape, int id);
    
    void update(float deltaTime) override;
    void moveLeft(float dt);
    void moveRight(float dt);
    
    bool intersects(const Ball& ball) const;
    
    float getWidth() const;
    float getHeight() const;
    float getSpeed() const;
    
    void setPosition(const glm::vec3& pos);
};
```

Key functionalities:
- Paddle movement
- Collision detection with ball
- Size and speed management

### Ball.hpp/cpp (Part of Breakout.hpp)

Represents the bouncing ball.

```cpp
class Ball : public GameObject {
private:
    float radius;
    glm::vec3 velocity;
    bool stuck;
    
public:
    Ball(const glm::vec3& pos, float radius, const Shape& shape, int id);
    
    void update(float deltaTime) override;
    
    void launch();
    void setStuck(bool stuck);
    bool isStuck() const;
    
    void stickToPaddle(const Paddle& paddle);
    
    void setVelocity(const glm::vec3& vel);
    glm::vec3 getVelocity() const;
    
    void reverseX();
    void reverseY();
    
    float getRadius() const;
};
```

Key functionalities:
- Ball movement and physics
- Collision response
- Attachment to paddle
- Velocity management

### Brick.hpp/cpp (Part of Breakout.hpp)

Represents a destructible brick.

```cpp
class Brick : public GameObject {
private:
    float width;
    float height;
    bool destroyed;
    int hitPoints;
    int scoreValue;
    
public:
    Brick(const glm::vec3& pos, float width, float height, int hitPoints, int scoreValue, const Shape& shape, int id);
    
    void update(float deltaTime) override;
    
    bool intersects(const Ball& ball) const;
    
    bool isDestroyed() const;
    void hit();
    
    float getWidth() const;
    float getHeight() const;
    int getScoreValue() const;
};
```

Key functionalities:
- Brick state management (hit points, destruction)
- Collision detection with ball
- Score value tracking

## Rendering

### Shape.hpp/cpp

Represents a 3D mesh with vertex data, normals, and OpenGL buffers.

```cpp
class Shape {
private:
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> norm;
    GLuint vao;
    GLuint vbo;
    
public:
    Shape(const size_t triangleCount, const std::vector<float>& vertexData);
    ~Shape();
    
    GLuint getVAO() const;
    GLuint getVBO() const;
    int getVertexCount() const;
};

// Helper functions
Shape createQuadShape();
Shape createCircleShape(int segments = 16);
bool loadMeshData(const std::string& filename, size_t& triangleCount, std::vector<float>& vertexData);
```

Key functionalities:
- OpenGL buffer management
- Vertex data storage
- Shape creation helpers for quads and circles
- Mesh loading from files

### Renderer.hpp/cpp

Handles the rendering of game objects.

```cpp
class Renderer {
private:
    std::vector<GameObject*> renderQueue;
    GLuint defaultTexture;
    
    void initializeDefaultTexture();
    
public:
    Renderer();
    ~Renderer();
    
    void submit(GameObject* object);
    void render(Shader& shader, const glm::mat4& view, const glm::mat4& proj);
};
```

Key functionalities:
- Render queue management
- Default texture initialization
- Object submission for rendering
- Batch rendering with shader and matrices

### Shader.hpp/cpp

Manages OpenGL shader programs.

```cpp
class Shader {
private:
    GLuint program;
    
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    void use() const;
    GLint getUniform(const std::string& name) const;
};
```

Key functionalities:
- Shader compilation and linking
- Shader program usage
- Uniform location lookup

### Camera.hpp/cpp

Represents a camera in 3D space.

```cpp
class Camera {
private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    bool matricesDirty;
    
    void updateMatrices();
    
public:
    Camera(float fov, float aspectRatio, float nearPlane, float farPlane);
    
    void setPosition(const glm::vec3& pos);
    const glm::vec3& getPosition() const;
    
    void setTarget(const glm::vec3& target);
    const glm::vec3& getTarget() const;
    
    void setUp(const glm::vec3& up);
    const glm::vec3& getUp() const;
    
    void setFOV(float fov);
    float getFOV() const;
    
    void setAspectRatio(float aspectRatio);
    float getAspectRatio() const;
    
    void setNearPlane(float nearPlane);
    float getNearPlane() const;
    
    void setFarPlane(float farPlane);
    float getFarPlane() const;
    
    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getProjectionMatrix() const;
    
    glm::vec3 getForwardVector() const;
    glm::vec3 getRightVector() const;
};
```

Key functionalities:
- Camera position and orientation management
- View and projection matrix calculation
- FOV, aspect ratio, and clipping plane settings
- Camera vectors (forward, right)

### Quaternion.hpp/cpp

Represents a quaternion for 3D rotations.

```cpp
class Quaternion {
public:
    float w;
    float x;
    float y;
    float z;
    
    Quaternion(float w = 1.0f, float x = 0.0f, float y = 0.0f, float z = 0.0f);
    Quaternion(float angle, glm::vec3 axis);
    
    Quaternion operator*(const Quaternion& q) const;
    glm::vec3 operator*(const glm::vec3& v) const;
    
    Quaternion conjugate() const;
    void normalize();
    
    glm::mat4 toMatrix() const;
    
    float getAngle() const;
    glm::vec3 getAxis() const;
    
    void print() const;
};
```

Key functionalities:
- Quaternion construction and operations
- Rotation of vectors
- Conversion to/from angle-axis
- Conversion to rotation matrix
- Normalization and conjugation

## Audio

### SoundSystem.hpp/cpp

Manages sound loading and playback using SDL_mixer.

```cpp
class SoundSystem {
private:
    struct SoundState {
        Uint8* buffer;
        Uint32 length;
        Uint32 position;
        bool playing;
        bool autoFree;
    };
    
    struct Sound {
        Uint8* buffer;
        Uint32 length;
        
        Sound(Uint8* buffer, Uint32 length) : buffer(buffer), length(length) {}
        ~Sound() { SDL_FreeWAV(buffer); }
        
        Uint8* getBuffer() const { return buffer; }
        Uint32 getLength() const { return length; }
    };
    
    SDL_AudioSpec audioSpec;
    SDL_AudioDeviceID deviceID;
    Uint8* mixBuffer;
    Uint32 mixBufferSize;
    
    std::vector<std::unique_ptr<Sound>> sounds;
    std::vector<SoundState> activeSounds;
    
    static void audioCallback(void* userdata, Uint8* stream, int len);
    
public:
    SoundSystem();
    ~SoundSystem();
    
    int loadSound(const std::string& filepath);
    void playSound(int soundIndex);
    void playSound(const std::string& filepath);
    void cleanup();
};
```

Key functionalities:
- Sound loading from WAV files
- Sound playback and mixing
- Active sound management
- Audio callback handling
- Cleanup of finished sounds

## Scene Management

### SceneGraph.hpp/cpp

Manages the spatial relationships between game objects.

```cpp
class SceneGraph {
private:
    std::unique_ptr<SceneNode> rootNode;
    std::unique_ptr<OctreeNode> octreeRoot;
    AABB worldBounds;
    
public:
    SceneGraph(const AABB& worldBounds);
    
    void addObject(GameObject* obj, SceneNode* parent = nullptr);
    void removeObject(GameObject* obj);
    
    void updateTransforms();
    void render(Renderer& renderer, const Camera& camera);
    
    SceneNode* createNode(SceneNode* parent = nullptr);
    void destroyNode(SceneNode* node);
    
    SceneNode* getRootNode() const;
    
    void getVisibleObjects(std::vector<GameObject*>& visibleObjects, const Camera& camera);
    void updateSpatialStructure();
};
```

Key functionalities:
- Scene hierarchy management
- Object addition and removal
- Transform updates
- Frustum culling
- Spatial partitioning using octree
- Visibility determination

### SceneNode.hpp/cpp

Represents a node in the scene graph.

```cpp
class SceneNode {
protected:
    SceneNode* parent;
    glm::mat4 localTransform;
    glm::mat4 worldTransform;
    bool transformDirty;
    
    AABB localBounds;
    AABB worldBounds;
    bool boundsDirty;
    
    std::vector<std::unique_ptr<SceneNode>> children;
    std::vector<GameObject*> objects;
    
public:
    SceneNode();
    
    void setLocalTransform(const glm::mat4& transform);
    const glm::mat4& getLocalTransform() const;
    const glm::mat4& getWorldTransform();
    
    void updateWorldTransform();
    void updateWorldBounds();
    
    const AABB& getWorldBounds();
    void setLocalBounds(const AABB& bounds);
    const AABB& getLocalBounds() const;
    
    void addChild(std::unique_ptr<SceneNode> child);
    std::unique_ptr<SceneNode> removeChild(SceneNode* child);
    
    void setParent(SceneNode* newParent);
    SceneNode* getParent() const;
    
    void attachObject(GameObject* obj);
    void detachObject(GameObject* obj);
    const std::vector<GameObject*>& getObjects() const;
    
    void render(Renderer& renderer, const Frustum& frustum);
    void update(float deltaTime);
    void collectVisibleObjects(std::vector<GameObject*>& visibleObjects, const Frustum& frustum);
};
```

Key functionalities:
- Local and world transform management
- Parent-child relationships
- Object attachment and detachment
- Bounds calculation
- Rendering and updating of attached objects
- Visibility determination

### OctreeNode.hpp/cpp

Represents a node in an octree spatial partitioning structure.

```cpp
class OctreeNode {
private:
    AABB bounds;
    int maxDepth;
    int currentDepth;
    int maxObjectsPerNode;
    bool isLeaf;
    OctreeNode* parent;
    
    std::unique_ptr<OctreeNode> children[8];
    std::vector<GameObject*> objects;
    
    void split();
    int getOctantForPoint(const glm::vec3& point) const;
    
public:
    OctreeNode(const AABB& bounds, int maxDepth = 8, int maxObjects = 16, int currentDepth = 0, OctreeNode* parent = nullptr);
    
    void insert(GameObject* obj);
    void remove(GameObject* obj);
    void update(GameObject* obj);
    
    void collectVisibleObjects(std::vector<GameObject*>& visibleObjects, const Frustum& frustum);
    void clear();
    
    const AABB& getBounds() const;
};
```

Key functionalities:
- Dynamic spatial partitioning
- Object insertion, removal, and updating
- Visibility determination
- Node splitting and merging
- Octant calculation for points

### AABB.hpp/cpp

Represents an axis-aligned bounding box.

```cpp
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    
    AABB(const glm::vec3& min = glm::vec3(0.0f), const glm::vec3& max = glm::vec3(0.0f));
    
    bool contains(const glm::vec3& point) const;
    bool intersects(const AABB& other) const;
    
    glm::vec3 getCenter() const;
    glm::vec3 getExtents() const;
};
```

Key functionalities:
- Bounding box representation
- Point containment testing
- Intersection testing with other AABBs
- Center and extents calculation

### Frustum.hpp/cpp

Represents a view frustum for culling.

```cpp
class Frustum {
public:
    enum Planes {
        Left = 0,
        Right,
        Bottom,
        Top,
        Near,
        Far,
        PlaneCount
    };
    
    glm::vec4 planes[PlaneCount];
    
    void updateFromCamera(const Camera& camera);
    
    bool containsPoint(const glm::vec3& point) const;
    bool containsSphere(const glm::vec3& center, float radius) const;
    bool containsAABB(const AABB& aabb) const;
};
```

Key functionalities:
- Frustum plane extraction from camera
- Point, sphere, and AABB containment testing
- Visibility determination for culling

## Game Flow

The game flow follows these steps:

1. **Initialization**:
   - SDL and OpenGL are initialized
   - Window is created
   - Sounds are loaded
   - Game objects are created
   - Scene graph is set up

2. **Main Loop**:
   - Engine updates (delta time calculation)
   - Events are processed
   - Game state is updated based on input
   - Physics and collision detection are applied
   - Objects are rendered
   - Audio is played as needed
   - Window is updated

3. **Game States**:
   - **Start**: Ball is attached to paddle, waiting for player to press space
   - **Playing**: Ball is moving, player controls paddle, bricks are being destroyed
   - **GameOver**: Player lost all lives, game can be restarted
   - **Win**: All bricks are destroyed, game can be restarted

4. **Game Mechanics**:
   - **Paddle Movement**: Player controls paddle with left/right keys
   - **Ball Physics**: Ball moves and bounces off walls, paddle, and bricks
   - **Brick Destruction**: Bricks are damaged when hit and destroyed when hit points reach zero
   - **Lives**: Player loses a life when ball falls below the bottom boundary
   - **Score**: Player earns points for destroying bricks
   - **Win Condition**: All bricks are destroyed
   - **Lose Condition**: All lives are lost

## Conclusion

This Breakout implementation demonstrates a complete game architecture with rendering, physics, audio, and scene management systems. The modular design allows for easy extension and modification of game features and behavior.
