# Mesh Animation System Documentation

## System Overview

This document outlines the design and implementation of the mesh animation system developed for our 3D game engine. The system supports skeletal animations with bone hierarchies, keyframe interpolation, and provides a flexible API for animation playback control.

## Architecture Flow Chart

```
┌─────────────────┐     ┌───────────────────┐     ┌────────────────┐
│  Animation Data │────▶│ Animation Manager │────▶│ AnimationPlayer│
│  (.anim files)  │     │                   │     │                │
└─────────────────┘     └───────────────────┘     └────────┬───────┘
                                                           │
┌─────────────────┐                                        │
│    Shape with   │◀───────────────────────────────────────┘
│    Armature     │         Updates bone transforms
└────────┬────────┘
         │
         │  Sends bone matrices to shader
         ▼
┌─────────────────┐
│     Renderer    │
└─────────────────┘
```

## Implementation Components

### 1. Data Structures

#### Animation
- Stores keyframes containing bone rotations at specific timestamps
- Provides interpolation between keyframes using quaternion SLERP
- Main data components:
  * Name
  * Duration
  * Keyframes list with timestamps, bone IDs, and rotations

#### Shape with Armature
- Extended the existing Shape class to support skeletal data
- Added bone hierarchy information with parent-child relationships
- Stores vertex weights for skinning
- Main additions:
  * Bone array with local positions and parent indices
  * Vertex-to-bone binding data with weights
  * Current bone matrices for transformation

### 2. Runtime Components

#### AnimationPlayer
- Controls animation playback for a specific GameObject
- Handles play, pause, stop, and seek operations
- Supports playback speed adjustment and looping
- Provides event callbacks for animation start, completion, and looping
- Updates target object's bone transforms based on current animation time

#### AnimationManager
- Central manager for all animations and players in the scene
- Loads animation data from files
- Creates and tracks AnimationPlayer instances for GameObjects
- Updates all active animations during the game loop

## Implementation Process

1. **Data Format Design**
   - Created a binary format for animation data
   - Defined structures for bone hierarchies and keyframes
   - Implemented file loading and parsing

2. **Shape Extension**
   - Extended the Shape class to store armature data
   - Added methods to update bone transforms
   - Modified the rendering pipeline to pass bone matrices to shaders

3. **Animation System**
   - Implemented interpolation between keyframes using quaternion SLERP
   - Created the AnimationPlayer class to control playback
   - Developed the AnimationManager to organize and update animations

4. **Integration with Engine**
   - Connected animation system to the main update loop
   - Added collision support for animated meshes
   - Integrated with existing GameObject system

## User API

The system provides a simple API for controlling animations:

```cpp
// Load an animation
animationManager.loadAnimation("Walk", "assets/walk.anim");

// Play animation on a character
animationManager.playAnimation(characterObject, "Walk", true);  // true = loop

// Get the player to adjust playback
AnimationPlayer* player = animationManager.getPlayer(characterObject);
player->setPlaybackSpeed(2.0f);  // Double speed
player->pause();
player->resume();
```

## Shader Integration

The animation system works with the shader through bone matrices:

```glsl
// Vertex shader excerpt
uniform mat4 boneMatrices[MAX_BONES];

void main() {
    // Apply bone transformations
    vec4 positionOS = vec4(0.0);
    for(int i = 0; i < 4; i++) {
        int boneIndex = int(boneIndices[i]);
        float weight = boneWeights[i];
        positionOS += boneMatrices[boneIndex] * vec4(position, 1.0) * weight;
    }
    
    // Transform to world space
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * positionOS;
}
```

## Technical Challenges and Solutions

1. **Quaternion Interpolation**
   - Challenge: Smooth interpolation between rotations
   - Solution: Implemented spherical linear interpolation (SLERP) for quaternions

2. **Bone Hierarchy**
   - Challenge: Correctly propagate transformations through bone hierarchies
   - Solution: Process bones in order based on parent-child relationships

3. **Performance Optimization**
   - Challenge: Efficient skinning for many animated objects
   - Solution: Implemented matrix caching and reduced redundant calculations

4. **Memory Management**
   - Challenge: Handling many animations without excessive memory usage
   - Solution: Reference-counted animation data that's shared between instances