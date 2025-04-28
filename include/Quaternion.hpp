#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>

class Quaternion {
private:
    float w, x, y, z;
    
public:
    // Default constructor - identity quaternion
    Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
    
    // Constructor from w, x, y, z components
    Quaternion(float w, float x, float y, float z);
    
    // Constructor from angle (in degrees) and axis
    Quaternion(float angle, glm::vec3 axis);
    
    // Comparison operators
    bool operator==(const Quaternion& other) const {
        return w == other.w && x == other.x && y == other.y && z == other.z;
    }
    
    bool operator!=(const Quaternion& other) const {
        return !(*this == other);
    }
    
    // Component accessors
    float getW() const { return w; }
    float getX() const { return x; }
    float getY() const { return y; }
    float getZ() const { return z; }
    
    // Quaternion operations
    Quaternion operator*(const Quaternion& q) const;
    glm::vec3 operator*(const glm::vec3& v) const;
    Quaternion conjugate() const;
    void normalize();
    
    // Convert to matrix
    glm::mat4 toMatrix() const;
    
    // Get angle and axis
    float getAngle() const;
    glm::vec3 getAxis() const;
    
    // Debug
    void print() const;

    glm::vec3 rotate(const glm::vec3& v) const;
    
    glm::vec3 inverseRotate(const glm::vec3& v) const;
};

#endif
