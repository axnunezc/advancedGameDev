#include "Animations.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>

// Animation Class Implementation
Animation::Animation() : duration(0.0f) {}

Animation Animation::loadFromFile(const std::string& filename) {
    Animation animation;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open animation file: " << filename << std::endl;
        return animation;
    }
    
    // Read animation name
    std::getline(file, animation.name);
    
    // Read number of keyframes
    int keyframeCount;
    file >> keyframeCount;
    
    // Read each keyframe
    for (int i = 0; i < keyframeCount; i++) {
        Keyframe keyframe;
        
        // Read timestamp
        file >> keyframe.timestamp;
        
        // Read number of bones
        int boneCount;
        file >> boneCount;
        
        // Read bone IDs and rotations
        for (int j = 0; j < boneCount; j++) {
            int boneID;
            float w, x, y, z;
            
            file >> boneID >> w >> x >> y >> z;
            
            keyframe.boneIDs.push_back(boneID);
            keyframe.rotations.push_back(Quaternion(w, x, y, z));
        }
        
        animation.keyframes.push_back(keyframe);
    }
    
    // Set animation duration
    if (!animation.keyframes.empty()) {
        animation.duration = animation.keyframes.back().timestamp;
    }
    
    file.close();
    std::cout << "Loaded animation: " << animation.name << " with " << keyframeCount 
              << " keyframes, duration: " << animation.duration << "s" << std::endl;
    
    return animation;
}

std::map<int, Quaternion> Animation::getBoneRotationsAtTime(float time) const {
    std::map<int, Quaternion> result;
    
    if (keyframes.empty()) {
        return result;
    }
    
    // Clamp time to animation duration
    time = std::max(0.0f, std::min(time, duration));
    
    // Handle special cases: time at start or beyond end
    if (time <= keyframes.front().timestamp) {
        // Before or at first keyframe - use first keyframe rotations
        for (size_t i = 0; i < keyframes.front().boneIDs.size(); i++) {
            result[keyframes.front().boneIDs[i]] = keyframes.front().rotations[i];
        }
        return result;
    }
    
    if (time >= keyframes.back().timestamp) {
        // At or beyond last keyframe - use last keyframe rotations
        for (size_t i = 0; i < keyframes.back().boneIDs.size(); i++) {
            result[keyframes.back().boneIDs[i]] = keyframes.back().rotations[i];
        }
        return result;
    }
    
    // Find the two keyframes to interpolate between
    const Keyframe* keyframe1 = nullptr;
    const Keyframe* keyframe2 = nullptr;
    
    for (size_t i = 0; i < keyframes.size() - 1; i++) {
        if (keyframes[i].timestamp <= time && time < keyframes[i + 1].timestamp) {
            keyframe1 = &keyframes[i];
            keyframe2 = &keyframes[i + 1];
            break;
        }
    }
    
    if (!keyframe1 || !keyframe2) {
        std::cerr << "Error: Could not find valid keyframes for time: " << time << std::endl;
        return result;
    }
    
    // Calculate interpolation factor (t) between 0 and 1
    float t = (time - keyframe1->timestamp) / (keyframe2->timestamp - keyframe1->timestamp);
    
    // Collect all unique bone IDs from both keyframes
    std::set<int> allBoneIDs;
    for (int id : keyframe1->boneIDs) allBoneIDs.insert(id);
    for (int id : keyframe2->boneIDs) allBoneIDs.insert(id);
    
    // Process each bone
    for (int boneID : allBoneIDs) {
        // Find bone in keyframe1
        auto it1 = std::find(keyframe1->boneIDs.begin(), keyframe1->boneIDs.end(), boneID);
        
        // Find bone in keyframe2
        auto it2 = std::find(keyframe2->boneIDs.begin(), keyframe2->boneIDs.end(), boneID);
        
        // If bone exists in both keyframes, interpolate between them
        if (it1 != keyframe1->boneIDs.end() && it2 != keyframe2->boneIDs.end()) {
            size_t index1 = std::distance(keyframe1->boneIDs.begin(), it1);
            size_t index2 = std::distance(keyframe2->boneIDs.begin(), it2);
            
            Quaternion q1 = keyframe1->rotations[index1];
            Quaternion q2 = keyframe2->rotations[index2];
            
            // Perform SLERP interpolation
            result[boneID] = slerp(q1, q2, t);
        }
        // If bone only exists in keyframe1, use its rotation
        else if (it1 != keyframe1->boneIDs.end()) {
            size_t index1 = std::distance(keyframe1->boneIDs.begin(), it1);
            result[boneID] = keyframe1->rotations[index1];
        }
        // If bone only exists in keyframe2, use its rotation
        else if (it2 != keyframe2->boneIDs.end()) {
            size_t index2 = std::distance(keyframe2->boneIDs.begin(), it2);
            result[boneID] = keyframe2->rotations[index2];
        }
    }
    
    return result;
}

Quaternion Animation::slerp(const Quaternion& q1, const Quaternion& q2, float t) {
    // Normalize quaternions
    Quaternion q1n = q1;
    Quaternion q2n = q2;
    q1n.normalize();
    q2n.normalize();
    
    // Calculate cosine of angle between quaternions
    float dot = q1n.getW() * q2n.getW() + q1n.getX() * q2n.getX() + q1n.getY() * q2n.getY() + q1n.getZ() * q2n.getZ();
    
    // If dot negative, adjust one quaternion to take shortest path
    if (dot < 0.0f) {
        q2n = Quaternion(-q2n.getW(), -q2n.getX(), -q2n.getY(), -q2n.getZ());
        dot = -dot;
    }
    
    // Clamp dot product to valid range to avoid NaN
    dot = glm::clamp(dot, -1.0f, 1.0f);
    
    // Set default values for limiting case
    float theta = acos(dot);
    float sinTheta = sin(theta);
    
    // If angle is small, use linear interpolation
    if (sinTheta < 0.001f) {
        return Quaternion(
            q1n.getW() * (1.0f - t) + q2n.getW() * t,
            q1n.getX() * (1.0f - t) + q2n.getX() * t,
            q1n.getY() * (1.0f - t) + q2n.getY() * t,
            q1n.getZ() * (1.0f - t) + q2n.getZ() * t
        );
    }
    
    // Calculate interpolation coefficients
    float s1 = sin((1.0f - t) * theta) / sinTheta;
    float s2 = sin(t * theta) / sinTheta;
    
    // Return interpolated quaternion
    return Quaternion(
        q1n.getW() * s1 + q2n.getW() * s2,
        q1n.getX() * s1 + q2n.getX() * s2,
        q1n.getY() * s1 + q2n.getY() * s2,
        q1n.getZ() * s1 + q2n.getZ() * s2
    );
}

// AnimationPlayer Implementation
AnimationPlayer::AnimationPlayer(GameObject* target) 
    : target(target), 
      currentAnimation(nullptr), 
      currentTime(0.0f), 
      isPlaying(false), 
      looping(false),
      playbackSpeed(1.0f) {}

void AnimationPlayer::play(Animation* animation, bool loop) {
    currentAnimation = animation;
    currentTime = 0.0f;
    isPlaying = true;
    looping = loop;
    
    // Apply initial pose immediately
    if (currentAnimation) {
        applyPoseAtCurrentTime();
    }
    
    // Trigger animation start event
    triggerEvent("onAnimationStart");
}

void AnimationPlayer::resume() {
    if (currentAnimation) {
        isPlaying = true;
    }
}

void AnimationPlayer::pause() {
    isPlaying = false;
}

void AnimationPlayer::stop() {
    isPlaying = false;
    currentTime = 0.0f;
    
    // Trigger animation stop event
    triggerEvent("onAnimationStop");
}

void AnimationPlayer::setPlaybackSpeed(float speed) {
    playbackSpeed = std::max(0.01f, speed);  // Prevent negative or zero speed
}

Animation* AnimationPlayer::getCurrentAnimation() const {
    return currentAnimation;
}

float AnimationPlayer::getCurrentTime() const {
    return currentTime;
}

bool AnimationPlayer::isAnimationPlaying() const {
    return isPlaying && currentAnimation != nullptr;
}

float AnimationPlayer::getCompletionPercentage() const {
    if (!currentAnimation || currentAnimation->duration <= 0.0f) {
        return 0.0f;
    }
    return std::min(1.0f, currentTime / currentAnimation->duration);
}

void AnimationPlayer::setTime(float time) {
    if (!currentAnimation) {
        return;
    }
    
    currentTime = std::max(0.0f, std::min(time, currentAnimation->duration));
    applyPoseAtCurrentTime();
}

void AnimationPlayer::registerEventCallback(const std::string& eventName, AnimEventCallback callback) {
    eventCallbacks[eventName] = callback;
}

void AnimationPlayer::update(float deltaTime) {
    if (!isPlaying || !currentAnimation) return;
    
    // Store previous time for event triggers
    float prevTime = currentTime;
    
    // Update time with speed factor
    currentTime += deltaTime * playbackSpeed;
    
    // Handle animation completion
    if (currentTime >= currentAnimation->duration) {
        // Trigger completion event before looping or stopping
        triggerEvent("onAnimationComplete");
        
        if (looping) {
            // For looping, wrap around to beginning
            currentTime = fmod(currentTime, currentAnimation->duration);
            triggerEvent("onAnimationLoop");
        } else {
            // For non-looping, clamp to end and stop
            currentTime = currentAnimation->duration;
            isPlaying = false;
        }
    }
    
    // Apply current pose
    applyPoseAtCurrentTime();
}

void AnimationPlayer::applyPoseAtCurrentTime() {
    if (!currentAnimation || !target) {
        return;
    }
    
    // Get interpolated bone rotations
    std::map<int, Quaternion> boneRotations = 
        currentAnimation->getBoneRotationsAtTime(currentTime);
    
    // Apply rotations to bones in target object
    target->updateBoneRotations(boneRotations);
}

void AnimationPlayer::triggerEvent(const std::string& eventName) {
    auto it = eventCallbacks.find(eventName);
    if (it != eventCallbacks.end() && it->second) {
        it->second(this, eventName);
    }
}

// AnimationManager Implementation
AnimationManager::~AnimationManager() {
    // Clean up animation players
    for (auto& pair : players) {
        delete pair.second;
    }
    players.clear();
}

bool AnimationManager::loadAnimation(const std::string& name, const std::string& filename) {
    Animation animation = Animation::loadFromFile(filename);
    if (animation.duration <= 0.0f) {
        std::cerr << "Failed to load animation: " << filename << std::endl;
        return false;
    }
    
    animations[name] = animation;
    return true;
}

Animation* AnimationManager::getAnimation(const std::string& name) {
    auto it = animations.find(name);
    if (it != animations.end()) {
        return &it->second;
    }
    return nullptr;
}

AnimationPlayer* AnimationManager::getPlayer(GameObject* object) {
    auto it = players.find(object);
    if (it != players.end()) {
        return it->second;
    }
    
    // Create new player if none exists
    AnimationPlayer* player = new AnimationPlayer(object);
    players[object] = player;
    return player;
}

void AnimationManager::playAnimation(GameObject* object, const std::string& animName, bool loop) {
    Animation* anim = getAnimation(animName);
    if (!anim) {
        std::cerr << "Animation not found: " << animName << std::endl;
        return;
    }
    
    AnimationPlayer* player = getPlayer(object);
    player->play(anim, loop);
}

void AnimationManager::update(float deltaTime) {
    for (auto& pair : players) {
        pair.second->update(deltaTime);
    }
}