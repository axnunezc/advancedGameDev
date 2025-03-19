#ifndef QUATERNION_HPP
#define QUATERNION_HPP

#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Quaternion {
private:
    float w, x, y, z;

public:
    // Default constructor (identity quaternion)
    Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}

    // Constructor using axis-angle representation
    Quaternion(float angle, glm::vec3 axis);

    // Direct initialization of quaternion components
    Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) {}

    Quaternion operator*(const Quaternion& q) const;
    glm::vec3 operator*(const glm::vec3& v) const;
    Quaternion conjugate() const;
    glm::mat4 toMatrix() const;
    void normalize();

    float getAngle() const;
    
    glm::vec3 getAxis() const;
    
    void print() const;
};

#endif
