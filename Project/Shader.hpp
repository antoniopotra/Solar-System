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

#include "Light.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace gps {

class Shader {
public:
    GLuint shaderProgram;
    void loadShader(std::string vertexShaderPath, std::string fragmentShaderPath);
    void useShaderProgram();
    void sendModelUniform(glm::mat4 modelMatrix);
    void sendViewUniform(glm::mat4 viewMatrix);
    void sendProjectionUniform(glm::mat4 projectionMatrix);
    void sendNormalMatrixUniform(glm::mat3 normalMatrix);
    void sendDirectionalLightUniform(gps::DirectionalLight light);
    void sendSunLightUniform(gps::PointLight sunLight);
    void init(std::string vertexShaderPath, std::string fragmentShaderPath);

private:
    GLint modelLocation;
    GLint viewLocation;
    GLint projectionLocation;
    GLint normalMatrixLocation;
    GLint lightDirectionLocation;
    GLint lightColorLocation;

    std::string readShaderFile(std::string file);
    void shaderCompileLog(GLuint shaderId);
    void shaderLinkLog(GLuint shaderProgramId);
    void initUniforms();
};

} // namespace gps
