#ifndef GJK_HPP
#define GJK_HPP

#include <glm/glm.hpp>
#include <vector>
#include "Quaternion.hpp"
#include "Shape.hpp"
#include "GameObject.hpp"

// Structure to store simplex points
class Simplex {
private:
    glm::vec3 points[4];
    int dimensions;

public:
    Simplex();
    
    void addPoint(const glm::vec3& point);
    int getDimensions() const;
    const glm::vec3& getLast() const;
    const glm::vec3& getPoint(int index) const;
    void setDimension(int dim);
    void clear();
};

// Structure to store GJK result
struct GJKResult {
    bool collision;
    float distance;  // Distance between shapes (0 if colliding)
    glm::vec3 closestPointA;  // Closest point on shape A to shape B
    glm::vec3 closestPointB;  // Closest point on shape B to shape A
    
    GJKResult() : collision(false), distance(0.0f), 
                  closestPointA(0.0f), closestPointB(0.0f) {}
};

namespace Collision {
    // Transform a point from local to world space
    glm::vec3 transformPoint(const glm::vec3& point, const Quaternion& rotation, const glm::vec3& position);
    
    // Support function for a single shape (finds furthest point in direction)
    glm::vec3 support(const Shape& shape, const Quaternion& rotation, const glm::vec3& position, 
                      const glm::vec3& direction);
    
    // Minkowski Difference support function
    glm::vec3 minkowskiSupport(const Shape& shapeA, const Quaternion& rotationA, const glm::vec3& positionA,
                               const Shape& shapeB, const Quaternion& rotationB, const glm::vec3& positionB,
                               const glm::vec3& direction);
    
    // Simplex processing functions
    bool checkLineCase(Simplex& simplex, glm::vec3& direction);
    bool checkTriangleCase(Simplex& simplex, glm::vec3& direction);
    bool checkTetrahedronCase(Simplex& simplex, glm::vec3& direction);
    bool processSimplex(Simplex& simplex, glm::vec3& direction);
    
    // Enhanced GJK that returns distance when not colliding
    GJKResult GJK(Shape& shapeA, Quaternion& rotationA, const glm::vec3& positionA,
                 Shape& shapeB, Quaternion& rotationB, const glm::vec3& positionB);
    
    // Calculate closest point on a line segment to the origin
    glm::vec3 closestPointOnLineToOrigin(const glm::vec3& a, const glm::vec3& b, float& t);
    
    // Calculate closest point on a triangle to the origin
    glm::vec3 closestPointOnTriangleToOrigin(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, 
                                           glm::vec3& barycentric);
    
    // Compute the distance between closest points of two shapes
    // Updated to include output parameters for closest points
    float computeDistance(const Simplex& simplex, glm::vec3& closestA, glm::vec3& closestB);
    
    // Legacy function for backward compatibility
    float computeDistance(const Simplex& simplex);
    
    // Helper to check collision between two GameObjects
    bool checkCollision(GameObject* objA, GameObject* objB);
    
    // Enhanced helper to check collision and get distance
    GJKResult checkCollisionWithDistance(GameObject* objA, GameObject* objB);
}

#endif // GJK_HPP