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

#include "GeometryShader.h"
#include "Model3D.hpp"
#include "Shader.hpp"

#include <string>

namespace gps {

class Sun {
public:
    Sun(glm::vec3 initialPosition, long long rotationPeriod, glm::vec3 rotationAxis, float radius, std::string filename, gps::Shader *shader);

    glm::mat4 getModelByTime(long long currentSeconds);
    glm::vec3 getCurrentPosition(long long currentSeconds);
    float getRadius();
    void render(const glm::mat4 *baseModel, const glm::mat4 *viewMatrix, long long seconds);
    void renderDepthMap(const glm::mat4 *baseModel,
                        const glm::mat4 *viewMatrix,
                        long long currentSeconds,
                        gps::GeometryShader *geometryShader);

private:
    glm::vec3 initialPosition;
    long long rotationPeriod;
    glm::vec3 rotationAxis;
    float radius;
    gps::Shader *shader;
    gps::Model3D *object;

    float computeRotationAngle(long long currentSeconds);
};

} // namespace gps
