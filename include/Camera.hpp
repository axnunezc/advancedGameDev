#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;
    
    mutable glm::mat4 viewMatrix;
    mutable glm::mat4 projectionMatrix;
    mutable bool matricesDirty;
    
public:
    Camera(float fov = 60.0f, float aspectRatio = 1.33f, float nearPlane = 0.1f, float farPlane = 100.0f);
    
    // Position and orientation
    void setPosition(const glm::vec3& pos);
    const glm::vec3& getPosition() const;
    
    void setTarget(const glm::vec3& target);
    const glm::vec3& getTarget() const;
    
    void setUp(const glm::vec3& up);
    const glm::vec3& getUp() const;
    
    // Projection parameters
    void setFOV(float fov);
    float getFOV() const;
    
    void setAspectRatio(float aspectRatio);
    float getAspectRatio() const;
    
    void setNearPlane(float nearPlane);
    float getNearPlane() const;
    
    void setFarPlane(float farPlane);
    float getFarPlane() const;
    
    // Matrix access
    const glm::mat4& getViewMatrix() const;
    const glm::mat4& getProjectionMatrix() const;
    
    // Update matrices
    void updateMatrices();
    
    // Utility methods
    glm::vec3 getForwardVector() const;
    glm::vec3 getRightVector() const;
};

#endif // CAMERA_HPP