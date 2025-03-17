#ifndef QUAT_HPP
#define QUAT_HPP

#include <glm/glm.hpp>
#include <cmath>

class Quat {
public:
    float w, x, y, z;

    Quat();  // Default constructor
    Quat(const glm::vec3& axis, float angle);  // Axis-angle constructor
    Quat(float w, float x, float y, float z);  // Raw value constructor

    Quat operator*(const Quat& q) const;
    glm::vec3 operator*(const glm::vec3& v) const;
    Quat conjugate() const;
    glm::mat4 toMat4() const;
};

#endif
