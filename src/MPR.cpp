#define GLM_ENABLE_EXPERIMENTAL
#include "MPR.hpp"
#include "GJK.hpp"  // For minkowskiSupport and support functions
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

namespace Collision {
    // Function to determine if ray from interior point to origin passes through portal
    bool rayPassesThroughPortal(const Portal& portal, const glm::vec3& interior) {
        glm::vec3 dir = -interior; // Direction to origin
        
        // Compute the determinant (triple scalar product)
        float det = glm::dot(dir, glm::cross(portal.v1 - portal.v0, portal.v2 - portal.v0));
        
        // If determinant is positive, ray passes through the portal
        return det > 0;
    }
    
    // Refine the portal to create a more accurate representation
    void refinePortal(Portal& portal, const Shape& shapeA, const Quaternion& rotationA, const glm::vec3& positionA,
                      const Shape& shapeB, const Quaternion& rotationB, const glm::vec3& positionB) {
        // Compute portal normal (pointing toward origin)
        portal.normal = glm::normalize(glm::cross(portal.v1 - portal.v0, portal.v2 - portal.v0));
        
        // Ensure normal points toward origin
        if (glm::dot(portal.normal, -portal.v0) < 0) {
            portal.normal = -portal.normal;
        }
        
        // Find support point in the direction of the portal normal
        glm::vec3 support = minkowskiSupport(shapeA, rotationA, positionA, shapeB, rotationB, positionB, portal.normal);
        
        // Update the portal with the new support point
        portal.v2 = support;
    }
    
    // Check if support point is in front of the portal toward the origin
    bool isPointInFrontOfPortal(const Portal& portal, const glm::vec3& point) {
        return glm::dot(portal.normal, point - portal.v0) > 0;
    }
    
    // Find an interior point in the Minkowski Difference
    glm::vec3 findInteriorPoint(const Shape& shapeA, const Quaternion& rotationA, const glm::vec3& positionA,
                              const Shape& shapeB, const Quaternion& rotationB, const glm::vec3& positionB) {
        // Center of Minkowski Difference 
        glm::vec3 center = positionA - positionB;
        
        // Find support in the direction of the center
        glm::vec3 supportA = support(shapeA, rotationA, positionA, center);
        glm::vec3 supportB = support(shapeB, rotationB, positionB, -center);
        
        // Compute an interior point by mixing center with support direction
        return glm::mix(supportA - supportB, center, 0.5f);
    }
    
    // MPR collision detection algorithm
    bool MPR(const Shape& shapeA, const Quaternion& rotationA, const glm::vec3& positionA,
            const Shape& shapeB, const Quaternion& rotationB, const glm::vec3& positionB) {
        // Get an interior point of the Minkowski Difference
        glm::vec3 interior = findInteriorPoint(shapeA, rotationA, positionA, shapeB, rotationB, positionB);
        
        // If interior point is at the origin, we have a collision
        if (glm::length2(interior) < 0.0001f) {
            return true;
        }
        
        // Get support point in direction of origin
        glm::vec3 support0 = minkowskiSupport(shapeA, rotationA, positionA, shapeB, rotationB, positionB, -interior);
        
        // If origin is not past support point, no collision
        if (glm::dot(support0, -interior) < 0) {
            return false;
        }
        
        // Direction perpendicular to line from interior to support0, pointing toward origin
        glm::vec3 dir = glm::normalize(glm::cross(glm::cross(interior, support0), interior));
        
        // Get second support point
        glm::vec3 support1 = minkowskiSupport(shapeA, rotationA, positionA, shapeB, rotationB, positionB, dir);
        
        // Initialize portal 
        Portal portal;
        portal.v0 = interior;
        portal.v1 = support0;
        portal.v2 = support1;
        
        // Refine portal until it faces origin accurately
        for (int i = 0; i < 32; i++) {
            // Refine the portal
            refinePortal(portal, shapeA, rotationA, positionA, shapeB, rotationB, positionB);
            
            // Get new support point in direction of portal normal
            glm::vec3 support = minkowskiSupport(shapeA, rotationA, positionA, shapeB, rotationB, positionB, portal.normal);
            
            // If new support point is not significantly further than portal
            if (glm::abs(glm::dot(portal.normal, support - portal.v0)) < 0.0001f) {
                // Check if ray from interior to origin passes through the portal
                return rayPassesThroughPortal(portal, interior);
            }
            
            // If the new support point is in front of the portal toward the origin
            if (isPointInFrontOfPortal(portal, support)) {
                // Update portal with the new support point
                portal.v2 = support;
            } else {
                return false;
            }
        }
        
        // Maximum iterations reached, check current portal
        return rayPassesThroughPortal(portal, interior);
    }
    
    // Helper to check collision between two GameObjects using MPR
    bool checkCollisionMPR(GameObject* objA, GameObject* objB) {
        // First check AABB overlap for early-out
        if (!objA->getBoundingBox().overlaps(objB->getBoundingBox())) {
            return false;
        }
        
        // Get shape and transform data from objects
        const Shape& shapeA = objA->getShape();
        const Shape& shapeB = objB->getShape();
        
        const Quaternion& rotationA = objA->getRotation();
        const Quaternion& rotationB = objB->getRotation();
        
        const glm::vec3& positionA = objA->getPosition();
        const glm::vec3& positionB = objB->getPosition();
        
        // Run MPR algorithm
        return MPR(shapeA, rotationA, positionA, shapeB, rotationB, positionB);
    }
}