#include "Camera.hpp"

namespace gps {

const float Camera::DFEAULT_MOVE_SPEED = 0.01;
const float Camera::DEFAULT_ROTATION_RADIANS = 0.0314;

Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
    this->cameraPosition = cameraPosition;
    this->worldUpDirection = cameraUp;
    this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
    isAnimating = false;
    updateCameraRightUpDirection();
}

glm::mat4 Camera::getViewMatrix() {
    if (!isAnimating) {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    } else {
        return glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), cameraUpDirection);
    }
}

glm::vec3 Camera::getCameraPosition() {
    glm::vec3 res = cameraPosition;
    return res;
}

void Camera::move(MOVE_DIRECTION direction, float speed) {
    switch (direction) {
    case MOVE_LEFT:
        cameraPosition -= cameraRightDirection * speed;
        return;
    case MOVE_RIGHT:
        cameraPosition += cameraRightDirection * speed;
        return;
    case MOVE_FORWARD:
        cameraPosition += cameraFrontDirection * speed;
        return;
    case MOVE_BACKWARD:
        cameraPosition -= cameraFrontDirection * speed;
    }
}

// yaw - camera rotation around the y axis
// pitch - camera rotation around the x axis
void Camera::rotate(float pitch, float yaw) {
    glm::vec3 previousCameraFrontDirection = cameraFrontDirection;

    glm::mat4 pitchRotator = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), cameraRightDirection);
    cameraFrontDirection = pitchRotator * glm::vec4(cameraFrontDirection, 1.0);

    glm::mat4 yawRotator = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), cameraUpDirection);
    cameraFrontDirection = yawRotator * glm::vec4(cameraFrontDirection, 1.0);

    cameraFrontDirection = glm::normalize(cameraFrontDirection);
    if (cameraFrontDirection == worldUpDirection) {
        cameraFrontDirection = previousCameraFrontDirection;
    } else {
        updateCameraRightUpDirection();
    }
}

void Camera::animate(float angle) {
    static float orbitRadius = -500.0f;
    static bool approaching = false;
    const float FAR_ORBIT_RADIUS = -540.0f;
    const float NEAR_ORBIT_RADIUS = -460.0f;
    
    if (approaching) {
        orbitRadius += 1.0f;
    } else {
        orbitRadius -= 1.0f;
    }
    if (orbitRadius >= NEAR_ORBIT_RADIUS) {
        approaching = false;
    } else if (orbitRadius <= FAR_ORBIT_RADIUS) {
        approaching = true;
    }

    float x = orbitRadius * glm::cos(glm::radians(angle));
    float z = orbitRadius * glm::sin(glm::radians(angle));

    cameraPosition = glm::vec3(x, 0, z);
}

void Camera::updateCameraRightUpDirection() {
    cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, worldUpDirection));
    cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
}
} // namespace gps
