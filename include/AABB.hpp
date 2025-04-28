#ifndef AABB_HPP
#define AABB_HPP

#include <glm/glm.hpp>

class AABB {
public:
    glm::vec3 min;
    glm::vec3 max;
    
    // Default constructor
    AABB() : min(0.0f), max(0.0f) {}
    
    // Constructor from min and max points
    AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}
    
    // Get the center of the AABB
    glm::vec3 getCenter() const {
        return (min + max) * 0.5f;
    }
    
    // Get the extents (half-size) of the AABB
    glm::vec3 getExtents() const {
        return (max - min) * 0.5f;
    }
    
    // Check if a point is contained in the AABB
    bool contains(const glm::vec3& point) const {
        return (point.x >= min.x && point.x <= max.x &&
                point.y >= min.y && point.y <= max.y &&
                point.z >= min.z && point.z <= max.z);
    }
    
    // Check if this AABB overlaps another AABB
    bool overlaps(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }
    
    // Merge with another AABB
    AABB merge(const AABB& other) const {
        return AABB(
            glm::min(min, other.min),
            glm::max(max, other.max)
        );
    }
};

#endif // AABB_HPP