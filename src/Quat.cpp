#include "Quat.hpp"

Quat::Quat() : w(1), x(0), y(0), z(0) {}

Quat::Quat(const glm::vec3& axis, float angle) {
    glm::vec3 normAxis = glm::normalize(axis);
    float halfAngle = angle * 0.5f;
    float s = sin(halfAngle);
    w = cos(halfAngle);
    x = normAxis.x * s;
    y = normAxis.y * s;
    z = normAxis.z * s;
}

Quat::Quat(float _w, float _x, float _y, float _z) : w(_w), x(_x), y(_y), z(_z) {}

Quat Quat::operator*(const Quat& q) const {
    return Quat(
        w * q.w - x * q.x - y * q.y - z * q.z,
        w * q.x + x * q.w + y * q.z - z * q.y,
        w * q.y - x * q.z + y * q.w + z * q.x,
        w * q.z + x * q.y - y * q.x + z * q.w
    );
}

glm::vec3 Quat::operator*(const glm::vec3& v) const {
    Quat vecQuat(0, v.x, v.y, v.z);
    Quat res = (*this) * vecQuat * conjugate();
    return glm::vec3(res.x, res.y, res.z);
}

Quat Quat::conjugate() const {
    return Quat(w, -x, -y, -z);
}

glm::mat4 Quat::toMat4() const {
    return glm::mat4(
        1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * w * z, 2 * x * z + 2 * w * y, 0,
        2 * x * y + 2 * w * z, 1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * w * x, 0,
        2 * x * z - 2 * w * y, 2 * y * z + 2 * w * x, 1 - 2 * x * x - 2 * y * y, 0,
        0, 0, 0, 1
    );
}
