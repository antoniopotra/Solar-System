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

#include "Shader.hpp"
#include "stb_image.hpp"

#include <stdio.h>
#include <vector>

namespace gps {
class Skybox {
public:
    Skybox();
    void Load(std::vector<const GLchar *> cubeMapFaces);
    void Draw(gps::Shader shader, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
    GLuint GetTextureId();

private:
    GLuint skyboxVAO;
    GLuint skyboxVBO;
    GLuint cubemapTexture;
    GLuint LoadSkyBoxTextures(std::vector<const GLchar *> cubeMapFaces);
    void InitSkyBox();
};
} // namespace gps
