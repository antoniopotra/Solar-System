#include "SolarSystem.hpp"

namespace gps {
SolarSystem::SolarSystem() {
}

void SolarSystem::init(gps::Shader *earthShader, gps::Shader *sunShader) {
    this->earthShader = earthShader;
    this->sunShader = sunShader;
    initSpaceObjects();
}

void SolarSystem::render(const glm::mat4 *baseModel, const glm::mat4 *viewMatrix, long long seconds) {
    sun->render(baseModel, viewMatrix, seconds);
    earth->render(baseModel, viewMatrix, seconds);
    moon->render(baseModel, viewMatrix, seconds);
}

void SolarSystem::renderDepthMap(const glm::mat4 *baseModel,
                                 const glm::mat4 *viewMatrix,
                                 long long currentSeconds,
                                 gps::GeometryShader *geometryShader) {
    earth->renderDepthMap(baseModel, viewMatrix, currentSeconds, geometryShader);
    moon->renderDepthMap(baseModel, viewMatrix, currentSeconds, geometryShader);
}

void SolarSystem::initSpaceObjects() {
    glm::vec3 sunPosition(0, 0, 0);
    gps::Sun *sun = new gps::Sun(sunPosition, 27 * SECONDS_IN_DAY, glm::vec3(0, 1, 0), 174.085, "models/sun/sun.obj", sunShader);
    this->sun = sun;

    glm::vec3 earthPosition(149600 / DISTANCE_DIVIDER, 0, 0);
    gps::Earth *earth = new gps::Earth(earthPosition, 24 * SECONDS_IN_HOUR, glm::vec3(0, 1, 0), 365.2 * SECONDS_IN_DAY, glm::vec3(0, 1, 0), 6.371, "models/earth/earth.obj", earthShader);
    this->earth = earth;

    glm::vec3 moonPosition(384 / DISTANCE_DIVIDER + MOON_OFFSET, 0, 0);
    gps::Moon *moon = new gps::Moon(moonPosition, 27 * SECONDS_IN_DAY, glm::vec3(0, 1, 0), 27.322 * SECONDS_IN_DAY, glm::vec3(0, 1, 0), earth, 1.737, "models/moon/moon.obj", earthShader);
    this->moon = moon;
}

bool SolarSystem::hasLandedOnEarth(long long currentSeconds, glm::vec3 currentPosition) {
    if (length(earth->getCurrentPosition(currentSeconds) - currentPosition) < earth->getRadius() * 1.1) {
        return true;
    }
    return false;
}
} // namespace gps
