#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : position(0.0f, 0.0f, 5.0f),
      target(0.0f, 0.0f, 0.0f),
      up(0.0f, 1.0f, 0.0f),
      fov(fov),
      aspectRatio(aspectRatio),
      nearPlane(nearPlane),
      farPlane(farPlane),
      viewMatrix(1.0f),
      projectionMatrix(1.0f),
      matricesDirty(true) {
}

void Camera::setPosition(const glm::vec3& pos) {
    position = pos;
    matricesDirty = true;
}

const glm::vec3& Camera::getPosition() const {
    return position;
}

void Camera::setTarget(const glm::vec3& target) {
    this->target = target;
    matricesDirty = true;
}

const glm::vec3& Camera::getTarget() const {
    return target;
}

void Camera::setUp(const glm::vec3& up) {
    this->up = up;
    matricesDirty = true;
}

const glm::vec3& Camera::getUp() const {
    return up;
}

void Camera::setFOV(float fov) {
    this->fov = fov;
    matricesDirty = true;
}

float Camera::getFOV() const {
    return fov;
}

void Camera::setAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
    matricesDirty = true;
}

float Camera::getAspectRatio() const {
    return aspectRatio;
}

void Camera::setNearPlane(float nearPlane) {
    this->nearPlane = nearPlane;
    matricesDirty = true;
}

float Camera::getNearPlane() const {
    return nearPlane;
}

void Camera::setFarPlane(float farPlane) {
    this->farPlane = farPlane;
    matricesDirty = true;
}

float Camera::getFarPlane() const {
    return farPlane;
}

const glm::mat4& Camera::getViewMatrix() const {
    if (matricesDirty) {
        const_cast<Camera*>(this)->updateMatrices();
    }
    return viewMatrix;
}

const glm::mat4& Camera::getProjectionMatrix() const {
    if (matricesDirty) {
        const_cast<Camera*>(this)->updateMatrices();
    }
    return projectionMatrix;
}

void Camera::updateMatrices() {
    // Update view matrix
    viewMatrix = glm::lookAt(position, target, up);
    
    // Update projection matrix
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
    
    matricesDirty = false;
}

glm::vec3 Camera::getForwardVector() const {
    return glm::normalize(target - position);
}

glm::vec3 Camera::getRightVector() const {
    return glm::normalize(glm::cross(getForwardVector(), up));
}