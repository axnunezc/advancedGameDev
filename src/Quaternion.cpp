#include "Quaternion.hpp"

// Angle-axis constructor
Quaternion::Quaternion(float angle, glm::vec3 axis) {
    axis = glm::normalize(axis);
    float halfAngle = glm::radians(angle) * 0.5f;
    float s = sin(halfAngle);
    
    w = cos(halfAngle);
    x = axis.x * s;
    y = axis.y * s;
    z = axis.z * s;
    
    normalize();
}

// Raw components constructor
Quaternion::Quaternion(float w, float x, float y, float z)
    : w(w), x(x), y(y), z(z) {
    normalize();
}

// Quaternion Multiplication (Hamilton Product)
Quaternion Quaternion::operator*(const Quaternion& q) const {
    return Quaternion(
        w * q.w - x * q.x - y * q.y - z * q.z,
        w * q.x + x * q.w + y * q.z - z * q.y,
        w * q.y - x * q.z + y * q.w + z * q.x,
        w * q.z + x * q.y - y * q.x + z * q.w
    );
}

// Quaternion * Vector3 (Rotate a vector)
glm::vec3 Quaternion::operator*(const glm::vec3& v) const {
    Quaternion p(0.0f, v); // Convert vector to quaternion (real part = 0)
    Quaternion rotated = (*this) * p * this->conjugate();
    return glm::vec3(rotated.x, rotated.y, rotated.z);
}

// Compute the conjugate (inverse for unit quaternions)
Quaternion Quaternion::conjugate() const {
    return Quaternion(w, -x, -y, -z);
}

// Normalize quaternion to unit length
void Quaternion::normalize() {
    float mag = std::sqrt(w * w + x * x + y * y + z * z);
    if (mag > 0.00001f) {
        w /= mag;
        x /= mag;
        y /= mag;
        z /= mag;
    }
}

// Convert quaternion to 4x4 rotation matrix
glm::mat4 Quaternion::toMatrix() const {
    return glm::mat4(
        1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * w * z, 2 * x * z + 2 * w * y, 0,
        2 * x * y + 2 * w * z, 1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * w * x, 0,
        2 * x * z - 2 * w * y, 2 * y * z + 2 * w * x, 1 - 2 * x * x - 2 * y * y, 0,
        0, 0, 0, 1
    );
}

float Quaternion::getAngle() const {
    return glm::degrees(2.0f * std::acos(w)); // Convert from quaternion representation to degrees
}

glm::vec3 Quaternion::getAxis() const {
    float s = std::sqrt(1 - w * w);
    return (s < 0.0001f) ? glm::vec3(1, 0, 0) : glm::vec3(x / s, y / s, z / s);
}


// Debug print function
void Quaternion::print() const {
    std::cout << "Quaternion(" << w << ", " << x << ", " << y << ", " << z << ")\n";
}

glm::vec3 Quaternion::rotate(const glm::vec3& v) const {
    // Convert quaternion to rotation matrix and apply to vector
    return glm::mat3_cast(glm::quat(w, x, y, z)) * v;
}

glm::vec3 Quaternion::inverseRotate(const glm::vec3& v) const {
    // Create inverse quaternion and use it to rotate
    Quaternion inverse(w, -x, -y, -z);
    return inverse.rotate(v);
}