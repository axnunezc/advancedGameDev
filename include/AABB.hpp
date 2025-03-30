#ifndef AABB_HPP
#define AABB_HPP

#include <glm/glm.hpp>

// Axis-Aligned Bounding Box
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    
    AABB() : min(0.0f), max(0.0f) {}
    AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
    
    bool contains(const glm::vec3& point) const {
        return (point.x >= min.x && point.x <= max.x &&
                point.y >= min.y && point.y <= max.y &&
                point.z >= min.z && point.z <= max.z);
    }
    
    bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x &&
                min.y <= other.max.y && max.y >= other.min.y &&
                min.z <= other.max.z && max.z >= other.min.z);
    }
    
    glm::vec3 getCenter() const {
        return (min + max) * 0.5f;
    }
    
    glm::vec3 getExtents() const {
        return (max - min) * 0.5f;
    }
};

#endif // AABB_HPP