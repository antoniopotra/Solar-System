#pragma once

#if defined(__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

namespace gps {

enum MOVE_DIRECTION {
    MOVE_FORWARD,
    MOVE_BACKWARD,
    MOVE_RIGHT,
    MOVE_LEFT
};

class Camera {
public:
    Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
    glm::mat4 getViewMatrix();
    void move(MOVE_DIRECTION direction, float speed = DFEAULT_MOVE_SPEED);
    // yaw - camera rotation around the y axis
    // pitch - camera rotation around the x axis
    void rotate(float pitch, float yaw);
    void animate(float angle);
    
    glm::vec3 getCameraPosition();
    
    bool isAnimating;

    static const float DFEAULT_MOVE_SPEED;
    static const float DEFAULT_ROTATION_RADIANS;
    
private:
    glm::vec3 cameraPosition;
    glm::vec3 cameraFrontDirection;
    glm::vec3 cameraRightDirection;
    glm::vec3 cameraUpDirection;
    glm::vec3 worldUpDirection;

    void updateCameraRightUpDirection();
};

} // namespace gps
