# Collision Testing and Event-Driven Response System

## Overview

This document outlines the implementation of a collision detection and response system for a game engine. The system focuses on efficient broad-phase collision detection using spatial partitioning and an event-driven response mechanism based on object types.

## System Architecture
```
┌─────────────────────┐      ┌───────────────────┐      ┌───────────────────┐
│                     │      │                   │      │                   │
│  Game Engine Loop   │──►   │   Collision       │──►   │  Type-based       │
│                     │      │   Detection       │      │  Event Response   │
│                     │      │                   │      │                   │
└─────────────────────┘      └───────────────────┘      └───────────────────┘
```

## Components

### 1. Type Registry System

The TypeRegistry provides a way to assign unique identifiers to GameObject types:
```
┌──────────────────┐
│  TypeRegistry    │
├──────────────────┤
│ + getInstance()  │
│ + registerType() │
│ + getTypeId()    │
│ + getNumTypes()  │
└──────────────────┘
```

- **Purpose**: Assign monotonically increasing IDs to each GameObject type
- **Implementation**: Singleton pattern with a hash map to store type names and IDs
- **Interface**: Provides methods to register types and retrieve type IDs

### 2. Spatial Partitioning (Octree)

The Octree structure organizes game objects spatially to accelerate collision detection:
```
┌──────────────────┐
│   OctreeNode     │
├──────────────────┤
│ + insert()       │
│ + remove()       │
│ + update()       │
│ + split()        │◄──┐
│                  │   │
└────────┬─────────┘   │
          │
          │ contains
          ▼
┌──────────────────┐   │
│  Child Nodes[8]  │───┘
└──────────────────┘
```

- **Purpose**: Reduce collision checks from O(n²) to O(n log n)
- **Implementation**: Recursive spatial subdivision with dynamic object redistribution
- **Interface**: Methods to insert, remove, update, and query objects

### 3. Collision Responder

The CollisionResponder manages collision callbacks based on object types:
```
┌───────────────────────┐
│  CollisionResponder   │
├───────────────────────┤
│ + registerCallback()  │
│ + processCollision()  │
│ + resize()            │
└───────────────────────┘
          │
          │ contains
          ▼
┌───────────────────────┐
│  Triangular Callback  │
│       Table           │
└───────────────────────┘
```

- **Purpose**: Store and invoke appropriate callbacks when objects collide
- **Implementation**: Triangular array for memory-efficient storage of collision functions
- **Interface**: Methods to register and process collision callbacks

### 4. Scene Graph Integration

The SceneGraph integrates collision detection and response:
```
┌───────────────────────────┐
│        SceneGraph         │
├───────────────────────────┤
│ + detectCollisions()      │
│ + registerCollisionCall.. │
│ + processCollisionResp..  │
└───────────────────────────┘
          │
          │ contains
          ▼
┌─────────────────┐    ┌───────────────┐
│   OctreeRoot    │    │ CollisionResp.│
└─────────────────┘    └───────────────┘
```

- **Purpose**: Tie together scene structure, collision detection, and event handling
- **Implementation**: Methods that use the octree for collision detection and the responder for event handling
- **Interface**: High-level methods for game code to interact with the collision system

## Data Flow

1. **Type Registration**: Each GameObject type is assigned a unique ID at registration time

2. **Collision Detection**:
   - Game objects are organized in the octree based on their AABB positions
   - The octree is queried to find potentially colliding object pairs
   - For each potential collision, AABB overlap tests are performed

3. **Collision Response**:
   - When a collision is detected, the type IDs of both objects are used
   - The collision responder looks up the appropriate callback in its table
   - The callback function is invoked if one exists for the given type pair

## Collision Detection Algorithm
```cpp
detectCollisions(objects):
    collisions = empty list
    for each object A in objects:
        potentialCollisions = empty list
        
        // Query the octree for potential collisions
        checkNode(octreeRoot, A.boundingBox, potentialCollisions)
        
        for each object B in potentialCollisions:
            if A < B:  // Avoid duplicates
                if A.boundingBox.overlaps(B.boundingBox):
                    collisions.add(pair(A, B))

    return collisions
```

## Event-Driven Response Mechanism
```cpp
processCollisionResponses():
    collisions = detectCollisions()
    for each (objectA, objectB) in collisions:
        typeA = objectA.getTypeId()
        typeB = objectB.getTypeId()
        
        callback = lookupCallback(typeA, typeB)
        
        if callback exists:
            callback(objectA, objectB)
```

## Memory-Efficient Callback Table

The collision callbacks are stored in a triangular array to avoid redundancy:
```cpp
getIndex(typeA, typeB):
    // Ensure typeA <= typeB to always access the same cell
    if typeA > typeB:
        swap(typeA, typeB)
    // Formula for triangular array indexing
    return (typeB * (typeB + 1)) / 2 + typeA
```

## Usage Example

```cpp
// Define game object types
class Player : public GameObject {
    DECLARE_GAMEOBJECT_TYPE()
    // ...
};

class Enemy : public GameObject {
    DECLARE_GAMEOBJECT_TYPE()
    // ...
};

// Register collision callback
sceneGraph.registerCollisionCallback(
    player->getTypeId(), 
    enemy->getTypeId(),
    [](GameObject* a, GameObject* b) {
        // Handle player-enemy collision
        Player* player = dynamic_cast<Player*>(a);
        Enemy* enemy = dynamic_cast<Enemy*>(b);
        
        // Game-specific response
        player->takeDamage(enemy->getDamage());
    }
);

// In game loop
sceneGraph.processCollisionResponses();
```

## Performance Considerations

- **Spatial Partitioning**: Reduces collision checks from O(n²) to O(n log n)
- **Type-Based Dispatch**: Constant-time lookup for collision callbacks
- **Memory Efficiency**: Triangular storage reduces memory usage by ~50%
- **Dynamic Updates**: Objects are automatically redistributed in the octree as they move

This collision system provides an efficient and flexible way to handle game object interactions while maintaining the separation of collision detection and game-specific response logic.
