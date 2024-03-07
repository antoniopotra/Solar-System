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

#include "Earth.hpp"
#include "GeometryShader.h"
#include "Model3D.hpp"
#include "Moon.hpp"
#include "Shader.hpp"
#include "Sun.hpp"

#include <memory>
#include <string>
#include <vector>

namespace gps {
class SolarSystem {
public:
    SolarSystem();

    void init(gps::Shader *earthShader, gps::Shader *sunShader);
    void render(const glm::mat4 *baseModel, const glm::mat4 *viewMatrix, long long seconds);
    void renderDepthMap(const glm::mat4 *baseModel,
                        const glm::mat4 *viewMatrix,
                        long long currentSeconds,
                        gps::GeometryShader *geometryShader);
    bool hasLandedOnEarth(long long currentSeconds, glm::vec3 currentPosition);

private:
    gps::Sun *sun;
    gps::Earth *earth;
    gps::Moon *moon;
    gps::Shader *earthShader;
    gps::Shader *sunShader;

    void initSpaceObjects();

    static const int SECONDS_IN_HOUR = 3600;
    static const int DISTANCE_DIVIDER = 500;
    static const int SECONDS_IN_DAY = SECONDS_IN_HOUR * 24;
    static const int MOON_OFFSET = 20;
};
} // namespace gps
