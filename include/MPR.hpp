#ifndef MPR_HPP
#define MPR_HPP

#include <glm/glm.hpp>
#include "Shape.hpp"
#include "GameObject.hpp"

namespace Collision {
    // MPR portal structure
    struct Portal {
        glm::vec3 v0, v1, v2;  // Portal vertices
        glm::vec3 normal;      // Normal of portal facing origin
    };
    
    // Function to determine if ray from interior point to origin passes through portal
    bool rayPassesThroughPortal(const Portal& portal, const glm::vec3& interior);
    
    // Refine the portal to create a more accurate representation
    void refinePortal(Portal& portal, const Shape& shapeA, const Quaternion& rotationA, const glm::vec3& positionA,
                      const Shape& shapeB, const Quaternion& rotationB, const glm::vec3& positionB);
    
    // Check if support point is in front of the portal toward the origin
    bool isPointInFrontOfPortal(const Portal& portal, const glm::vec3& point);
    
    // Find an interior point in the Minkowski Difference
    glm::vec3 findInteriorPoint(const Shape& shapeA, const Quaternion& rotationA, const glm::vec3& positionA,
                              const Shape& shapeB, const Quaternion& rotationB, const glm::vec3& positionB);
    
    // MPR collision detection algorithm
    bool MPR(const Shape& shapeA, const Quaternion& rotationA, const glm::vec3& positionA,
            const Shape& shapeB, const Quaternion& rotationB, const glm::vec3& positionB);
    
    // Helper to check collision between two GameObjects using MPR
    bool checkCollisionMPR(GameObject* objA, GameObject* objB);
}

#endif // MPR_HPP