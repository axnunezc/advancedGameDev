#define GLM_ENABLE_EXPERIMENTAL
#include "GJK.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <limits>

// Simplex Implementation
Simplex::Simplex() : dimensions(0) {}

void Simplex::addPoint(const glm::vec3& point) {
    // Shift existing points if we have 4 already
    if (dimensions == 4) {
        points[0] = points[1];
        points[1] = points[2];
        points[2] = points[3];
        points[3] = point;
    } else {
        points[dimensions] = point;
        dimensions++;
    }
}

int Simplex::getDimensions() const {
    return dimensions;
}

const glm::vec3& Simplex::getLast() const {
    return points[dimensions - 1];
}

const glm::vec3& Simplex::getPoint(int index) const {
    return points[index];
}

void Simplex::setDimension(int dim) {
    dimensions = dim;
}

void Simplex::clear() {
    dimensions = 0;
}

namespace Collision {
    // Transform a point from local to world space
    glm::vec3 transformPoint(const glm::vec3& point, const Quaternion& rotation, const glm::vec3& position) {
        // Use quaternion to rotate point and then add position
        return rotation.rotate(point) + position;
    }

    // Support function for a single shape (finds furthest point in direction)
    glm::vec3 support(const Shape& shape, const Quaternion& rotation, const glm::vec3& position, 
                      const glm::vec3& direction) {
        // Transform direction to local space
        glm::vec3 localDir = rotation.inverseRotate(direction);
        
        // Find furthest point in local space
        const std::vector<glm::vec3>& vertices = shape.getPositions();
        glm::vec3 furthestPoint = vertices[0];
        float maxDot = glm::dot(localDir, vertices[0]);
        
        for (size_t i = 1; i < vertices.size(); i++) {
            float dot = glm::dot(localDir, vertices[i]);
            if (dot > maxDot) {
                maxDot = dot;
                furthestPoint = vertices[i];
            }
        }
        
        // Transform back to world space
        return transformPoint(furthestPoint, rotation, position);
    }

    // Minkowski Difference support function
    glm::vec3 minkowskiSupport(const Shape& shapeA, const Quaternion& rotationA, const glm::vec3& positionA,
                              const Shape& shapeB, const Quaternion& rotationB, const glm::vec3& positionB,
                              const glm::vec3& direction) {
        // Get furthest point of shape A in direction
        glm::vec3 pointA = support(shapeA, rotationA, positionA, direction);
        
        // Get furthest point of shape B in opposite direction
        glm::vec3 pointB = support(shapeB, rotationB, positionB, -direction);
        
        // Return Minkowski Difference (A-B)
        return pointA - pointB;
    }

    // Check if the origin is in the line segment
    bool checkLineCase(Simplex& simplex, glm::vec3& direction) {
        // Line AB is our simplex with A as the most recently added point
        const glm::vec3& a = simplex.getLast();     // Last point added
        const glm::vec3& b = simplex.getPoint(0);   // First point
        
        // Line AB
        glm::vec3 ab = b - a;
        // Line AO (to origin)
        glm::vec3 ao = -a;
        
        // If the origin is in the direction of AB perpendicular to AO
        if (glm::dot(ab, ao) > 0) {
            // Set direction perpendicular to AB towards origin
            direction = glm::cross(glm::cross(ab, ao), ab);
            return false;
        }
        
        // Origin can only be in the direction of AO
        simplex.setDimension(1); // Reduce to just point A
        direction = ao;          // Set direction towards origin
        return false;
    }

    // Check if origin is in triangle
    bool checkTriangleCase(Simplex& simplex, glm::vec3& direction) {
        const glm::vec3& a = simplex.getLast();     // Last point added
        const glm::vec3& b = simplex.getPoint(1);   // Second point
        const glm::vec3& c = simplex.getPoint(0);   // First point
        
        // Edges
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;
        
        // Face normal
        glm::vec3 faceNormal = glm::cross(ab, ac);
        
        // Vector from a to origin
        glm::vec3 ao = -a;
        
        // Check if origin is above the triangle
        if (glm::dot(glm::cross(ab, faceNormal), ao) > 0) {
            // Origin is outside on the AB side
            if (glm::dot(ab, ao) > 0) {
                // Reduce to line case
                simplex.setDimension(2);
                simplex.addPoint(b);
                simplex.addPoint(a);
                direction = glm::cross(glm::cross(ab, ao), ab);
                return false;
            }
            // Origin is outside on the AC side
            else if (glm::dot(ac, ao) > 0) {
                // Reduce to line case
                simplex.setDimension(2);
                simplex.addPoint(c);
                simplex.addPoint(a);
                direction = glm::cross(glm::cross(ac, ao), ac);
                return false;
            }
            // Origin is outside beyond A
            else {
                // Reduce to a single point
                simplex.setDimension(1);
                simplex.addPoint(a);
                direction = ao;
                return false;
            }
        }
        // Check if origin is below the triangle
        else if (glm::dot(glm::cross(faceNormal, ac), ao) > 0) {
            // Origin is outside on the AC side
            if (glm::dot(ac, ao) > 0) {
                // Reduce to line case
                simplex.setDimension(2);
                simplex.addPoint(c);
                simplex.addPoint(a);
                direction = glm::cross(glm::cross(ac, ao), ac);
                return false;
            }
            // Origin is outside beyond A
            else {
                // Reduce to a single point
                simplex.setDimension(1);
                simplex.addPoint(a);
                direction = ao;
                return false;
            }
        }
        // Origin is in the direction of the face normal
        else {
            direction = faceNormal;
            if (glm::dot(faceNormal, ao) < 0) {
                direction = -faceNormal;
            }
            return false;
        }
    }

    // Check if origin is in tetrahedron
    bool checkTetrahedronCase(Simplex& simplex, glm::vec3& direction) {
        const glm::vec3& a = simplex.getLast();     // Newest point
        const glm::vec3& b = simplex.getPoint(2);   // Third point
        const glm::vec3& c = simplex.getPoint(1);   // Second point
        const glm::vec3& d = simplex.getPoint(0);   // First point
        
        // Vectors from a to other points
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;
        glm::vec3 ad = d - a;
        glm::vec3 ao = -a;  // Vector to origin
        
        // Face normals pointing outward
        glm::vec3 abc = glm::cross(ab, ac);
        glm::vec3 acd = glm::cross(ac, ad);
        glm::vec3 adb = glm::cross(ad, ab);
        
        // Make sure normals point outward
        if (glm::dot(abc, ad) > 0) {
            abc = -abc;
        }
        if (glm::dot(acd, ab) > 0) {
            acd = -acd;
        }
        if (glm::dot(adb, ac) > 0) {
            adb = -adb;
        }
        
        // Check each face
        if (glm::dot(abc, ao) > 0) {
            // Origin outside ABC, reduce to triangle
            simplex.setDimension(3);
            simplex.addPoint(c);
            simplex.addPoint(b);
            simplex.addPoint(a);
            direction = abc;
            return false;
        } 
        else if (glm::dot(acd, ao) > 0) {
            // Origin outside ACD, reduce to triangle
            simplex.setDimension(3);
            simplex.addPoint(d);
            simplex.addPoint(c);
            simplex.addPoint(a);
            direction = acd;
            return false;
        } 
        else if (glm::dot(adb, ao) > 0) {
            // Origin outside ADB, reduce to triangle
            simplex.setDimension(3);
            simplex.addPoint(b);
            simplex.addPoint(d);
            simplex.addPoint(a);
            direction = adb;
            return false;
        } 
        
        // Origin is inside tetrahedron
        return true;
    }

    // Process the simplex to determine next direction
    bool processSimplex(Simplex& simplex, glm::vec3& direction) {
        switch (simplex.getDimensions()) {
            case 2:  // Line case
                return checkLineCase(simplex, direction);
            case 3:  // Triangle case
                return checkTriangleCase(simplex, direction);
            case 4:  // Tetrahedron case
                return checkTetrahedronCase(simplex, direction);
            default:
                return false;
        }
    }

    // Calculate closest point on a line segment to the origin
    glm::vec3 closestPointOnLineToOrigin(const glm::vec3& a, const glm::vec3& b, float& t) {
        glm::vec3 ab = b - a;
        // Project origin onto line
        t = glm::dot(-a, ab) / glm::dot(ab, ab);
        // Clamp t to [0,1] for line segment
        t = glm::clamp(t, 0.0f, 1.0f);
        // Return closest point
        return a + t * ab;
    }

    // Calculate closest point on a triangle to the origin
    glm::vec3 closestPointOnTriangleToOrigin(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, 
                                           glm::vec3& barycentric) {
        // Compute the closest point on each edge
        float t;
        glm::vec3 closestAB = closestPointOnLineToOrigin(a, b, t);
        float distAB = glm::length2(closestAB);
        
        glm::vec3 closestBC = closestPointOnLineToOrigin(b, c, t);
        float distBC = glm::length2(closestBC);
        
        glm::vec3 closestCA = closestPointOnLineToOrigin(c, a, t);
        float distCA = glm::length2(closestCA);
        
        // Find closest point among edges
        glm::vec3 closest = closestAB;
        float minDist = distAB;
        barycentric = glm::vec3(1.0f - t, t, 0.0f);
        
        if (distBC < minDist) {
            closest = closestBC;
            minDist = distBC;
            barycentric = glm::vec3(0.0f, 1.0f - t, t);
        }
        
        if (distCA < minDist) {
            closest = closestCA;
            minDist = distCA;
            barycentric = glm::vec3(t, 0.0f, 1.0f - t);
        }
        
        return closest;
    }

// Enhanced GJK that returns distance when not colliding
GJKResult GJK(Shape& shapeA, Quaternion& rotationA, const glm::vec3& positionA,
    Shape& shapeB, Quaternion& rotationB, const glm::vec3& positionB) {
GJKResult result;

// Initialize simplex
Simplex simplex;

// Initial direction
glm::vec3 direction = positionB - positionA;
if (glm::length2(direction) < 0.0001f) {
direction = glm::vec3(1.0f, 0.0f, 0.0f);  // Default direction if shapes are centered at same point
}

// Get first support point
glm::vec3 support = minkowskiSupport(shapeA, rotationA, positionA, shapeB, rotationB, positionB, direction);
simplex.addPoint(support);

// New direction towards origin
direction = -support;

// Store the actual support points from both shapes
glm::vec3 lastSupportA = Collision::support(shapeA, rotationA, positionA, direction);
glm::vec3 lastSupportB = Collision::support(shapeB, rotationB, positionB, -direction);

// Main loop
for (int i = 0; i < 32; i++) {  // Limit iterations to avoid infinite loops
// Get new support point
support = minkowskiSupport(shapeA, rotationA, positionA, shapeB, rotationB, positionB, direction);

// Store the actual support points from both shapes
glm::vec3 supportPointA = Collision::support(shapeA, rotationA, positionA, direction);
glm::vec3 supportPointB = Collision::support(shapeB, rotationB, positionB, -direction);

// Check if we can't move further towards origin
if (glm::dot(support, direction) < 0) {
   // No collision - compute distance
   glm::vec3 closestPointOnMinkDiff = glm::vec3(0.0f);  // Will be set by computeDistance
   glm::vec3 closestOnB = glm::vec3(0.0f);             // Will be set by computeDistance
   
   result.collision = false;
   result.distance = computeDistance(simplex, closestPointOnMinkDiff, closestOnB);
   
   // Calculate closest points on shapes A and B
   // Here we approximate using the last saved support points
   // For a more accurate implementation, we would need to track where each point
   // in the simplex came from in the original shapes
   
   // Approximate closest points (better than returning (0,0,0))
   float distA = glm::length(lastSupportA);
   float distB = glm::length(lastSupportB);
   
   if (distA < 0.001f) lastSupportA = positionA;
   if (distB < 0.001f) lastSupportB = positionB;
   
   result.closestPointA = lastSupportA;
   result.closestPointB = lastSupportB;
   
   return result;
}

// Add point to simplex
simplex.addPoint(support);

// Update last support points
lastSupportA = supportPointA;
lastSupportB = supportPointB;

// Check if origin is in simplex
if (processSimplex(simplex, direction)) {
   // Collision!
   result.collision = true;
   result.distance = 0.0f;
   
   // Store approximate penetration points
   result.closestPointA = lastSupportA;
   result.closestPointB = lastSupportB;
   
   return result;
}

// If direction is near zero, we're not converging
if (glm::length2(direction) < 0.0001f) {
   result.collision = false;
   
   // Compute distance using current simplex
   glm::vec3 closestPointOnMinkDiff = glm::vec3(0.0f);
   glm::vec3 closestOnB = glm::vec3(0.0f);
   result.distance = computeDistance(simplex, closestPointOnMinkDiff, closestOnB);
   
   // Approximate closest points
   result.closestPointA = lastSupportA;
   result.closestPointB = lastSupportB;
   
   return result;
}
}

// Maximum iterations reached
result.collision = false;

// Compute distance using current simplex
glm::vec3 closestPointOnMinkDiff = glm::vec3(0.0f);
glm::vec3 closestOnB = glm::vec3(0.0f);
result.distance = computeDistance(simplex, closestPointOnMinkDiff, closestOnB);

// Approximate closest points
result.closestPointA = lastSupportA;
result.closestPointB = lastSupportB;

return result;
}

float computeDistance(const Simplex& simplex, glm::vec3& closestA, glm::vec3& closestB) {
    // Default to a large distance if simplex is empty
    if (simplex.getDimensions() == 0) {
        return std::numeric_limits<float>::max();
    }

    // For a single point, distance is just the length
    if (simplex.getDimensions() == 1) {
    // The closest point on the Minkowski difference is the single point
    glm::vec3 point = simplex.getPoint(0);
    float distance = glm::length(point);

    // Store the closest point (will need to be transformed back to original shapes)
    // This is an approximation since we don't store original points
    closestA = glm::vec3(0.0f);
    closestB = -point;

    return distance;
    }

    // For a line segment, find the closest point on the line to the origin
    if (simplex.getDimensions() == 2) {
        const glm::vec3& a = simplex.getPoint(0);
        const glm::vec3& b = simplex.getPoint(1);
        glm::vec3 ab = b - a;

        // Project origin onto the line
        float t = glm::clamp(glm::dot(-a, ab) / glm::dot(ab, ab), 0.0f, 1.0f);
        glm::vec3 closestPoint = a + t * ab;

        // Store the closest point 
        closestA = glm::vec3(0.0f);
        closestB = -closestPoint;

        return glm::length(closestPoint);
    }

    // For a triangle, find the closest point on the triangle to the origin
    if (simplex.getDimensions() == 3) {
        const glm::vec3& a = simplex.getPoint(0);
        const glm::vec3& b = simplex.getPoint(1);
        const glm::vec3& c = simplex.getPoint(2);

        // Calculate barycentric coordinates for closest point
        glm::vec3 barycentric;
        glm::vec3 closestPoint = glm::vec3(0.0f);

        // Check if origin is above the triangle
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;
        glm::vec3 normal = glm::cross(ab, ac);
        normal = glm::normalize(normal);

        // Project origin onto triangle plane
        float dist = glm::dot(normal, a);
        glm::vec3 planePoint = normal * dist;

            // If very close to plane, check if projection is inside triangle
            if (std::abs(dist) < 0.001f) {
            // Origin projection calculation omitted for brevity
            // Would need to check if projection is inside triangle
            
            // For now, just use edge distances
            float tAB, tBC, tCA;
            glm::vec3 pointAB = closestPointOnLineToOrigin(a, b, tAB);
            glm::vec3 pointBC = closestPointOnLineToOrigin(b, c, tBC);
            glm::vec3 pointCA = closestPointOnLineToOrigin(c, a, tCA);
            
            float distAB = glm::length(pointAB);
            float distBC = glm::length(pointBC);
            float distCA = glm::length(pointCA);
    
            if (distAB <= distBC && distAB <= distCA) {
                closestPoint = pointAB;
            } else if (distBC <= distAB && distBC <= distCA) {
                closestPoint = pointBC;
            } else {
                closestPoint = pointCA;
            }
            } else {
                // Origin is not in plane, use closest point on edges
                float tAB, tBC, tCA;
                glm::vec3 pointAB = closestPointOnLineToOrigin(a, b, tAB);
                glm::vec3 pointBC = closestPointOnLineToOrigin(b, c, tBC);
                glm::vec3 pointCA = closestPointOnLineToOrigin(c, a, tCA);
                
                float distAB = glm::length(pointAB);
                float distBC = glm::length(pointBC);
                float distCA = glm::length(pointCA);
    
    if (distAB <= distBC && distAB <= distCA) {
        closestPoint = pointAB;
    } else if (distBC <= distAB && distBC <= distCA) {
        closestPoint = pointBC;
    } else {
        closestPoint = pointCA;
    }
    }

    // Store the closest point
    closestA = glm::vec3(0.0f);
    closestB = -closestPoint;

    return glm::length(closestPoint);
    }

    // Tetrahedron case (shouldn't happen in distance calculation)
    return std::numeric_limits<float>::max();
    }

    // Helper to check collision between two GameObjects
    bool checkCollision(GameObject* objA, GameObject* objB) {
        // First check AABB overlap for early-out
        if (!objA->getBoundingBox().overlaps(objB->getBoundingBox())) {
            return false;
        }
        
        // Get shape and transform data from objects
        Shape& shapeA = objA->getShape();
        Shape& shapeB = objB->getShape();
        
        Quaternion& rotationA = objA->getRotation();
        Quaternion& rotationB = objB->getRotation();
        
        const glm::vec3& positionA = objA->getPosition();
        const glm::vec3& positionB = objB->getPosition();
        
        // Run GJK algorithm and just return collision status
        GJKResult result = GJK(shapeA, rotationA, positionA, shapeB, rotationB, positionB);
        return result.collision;
    }
    
    // Enhanced helper to check collision and get distance
    GJKResult checkCollisionWithDistance(GameObject* objA, GameObject* objB) {
        // Default result (no collision, maximum distance)
        GJKResult result;
        result.collision = false;
        result.distance = std::numeric_limits<float>::max();
        
        // First check AABB overlap for early-out
        if (!objA->getBoundingBox().overlaps(objB->getBoundingBox())) {
            return result;
        }
        
        // Get shape and transform data from objects
        Shape& shapeA = objA->getShape();
        Shape& shapeB = objB->getShape();
        
        Quaternion& rotationA = objA->getRotation();
        Quaternion& rotationB = objB->getRotation();
        
        const glm::vec3& positionA = objA->getPosition();
        const glm::vec3& positionB = objB->getPosition();
        
        // Run GJK algorithm and return full result
        return GJK(shapeA, rotationA, positionA, shapeB, rotationB, positionB);
    }
}