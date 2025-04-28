#ifndef ANIMATIONS_HPP
#define ANIMATIONS_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include "GameObject.hpp"
#include "Quaternion.hpp"

// Forward declaration
class AnimationPlayer;

// Animation class to store keyframes and bone rotations
class Animation {
public:
    struct Keyframe {
        float timestamp;                   // Time in seconds
        std::vector<int> boneIDs;          // IDs of bones animated in this keyframe
        std::vector<Quaternion> rotations; // Rotations for each bone
    };
    
    Animation();
    
    // Load animation from file
    static Animation loadFromFile(const std::string& filename);
    
    // Get interpolated bone rotations at a specific time
    std::map<int, Quaternion> getBoneRotationsAtTime(float time) const;
    
    std::string name;
    std::vector<Keyframe> keyframes;
    float duration;  // Total animation length

private:
    // SLERP implementation for quaternion interpolation
    static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, float t);
};

// AnimationPlayer to manage animation playback
class AnimationPlayer {
public:
    // Event callback function type for animation events
    using AnimEventCallback = std::function<void(AnimationPlayer*, const std::string&)>;
    
    AnimationPlayer(GameObject* target);
    
    // Play animation from start
    void play(Animation* animation, bool loop = false);
    
    // Resume paused animation
    void resume();
    
    // Pause animation
    void pause();
    
    // Stop animation and reset time
    void stop();
    
    // Set playback speed (1.0 = normal speed)
    void setPlaybackSpeed(float speed);
    
    // Get current playback speed
    float getPlaybackSpeed() const { return playbackSpeed; }
    
    // Get current animation
    Animation* getCurrentAnimation() const;
    
    // Get current animation time
    float getCurrentTime() const;
    
    // Check if animation is playing
    bool isAnimationPlaying() const;
    
    // Get animation completion percentage (0.0 to 1.0)
    float getCompletionPercentage() const;
    
    // Set animation time directly (for seeking)
    void setTime(float time);
    
    // Register event callback
    void registerEventCallback(const std::string& eventName, AnimEventCallback callback);
    
    // Update animation state
    void update(float deltaTime);
    
private:
    GameObject* target;                // Target object with armature
    Animation* currentAnimation;       // Current animation being played
    float currentTime;                 // Current playback time
    bool isPlaying;
    bool looping;
    float playbackSpeed;               // Speed multiplier for animation
    
    std::map<std::string, AnimEventCallback> eventCallbacks;
    
    // Apply pose at current time
    void applyPoseAtCurrentTime();
    
    // Trigger an event callback
    void triggerEvent(const std::string& eventName);
};

// Animation Manager to handle multiple animations
class AnimationManager {
public:
    ~AnimationManager();
    
    // Load animation from file
    bool loadAnimation(const std::string& name, const std::string& filename);
    
    // Get animation by name
    Animation* getAnimation(const std::string& name);
    
    // Get or create animation player for object
    AnimationPlayer* getPlayer(GameObject* object);
    
    // Play animation on object
    void playAnimation(GameObject* object, const std::string& animName, bool loop = false);
    
    // Update all animation players
    void update(float deltaTime);
    
private:
    std::map<std::string, Animation> animations;
    std::map<GameObject*, AnimationPlayer*> players;
};

#endif // ANIMATIONS_HPP