#include "Moon.hpp"

namespace gps {
Moon::Moon(glm::vec3 initialPosition, long long rotationPeriod, glm::vec3 rotationAxis, long long orbitPeriod, glm::vec3 orbitAxis, Earth *earth, float radius, std::string filename, gps::Shader *shader) {
    this->initialPosition = initialPosition;
    this->rotationPeriod = rotationPeriod;
    this->rotationAxis = rotationAxis;
    this->orbitPeriod = orbitPeriod;
    this->orbitAxis = orbitAxis;
    this->earth = earth;
    this->radius = radius;
    this->object = new gps::Model3D();
    this->object->LoadModel(filename);
    this->shader = shader;
}

glm::mat4 Moon::getModelByTime(long long currentSeconds) {
    glm::mat4 model(1.0);

    glm::vec3 currentEarthPosition = earth->getCurrentPosition(currentSeconds);
    model = glm::translate(model, currentEarthPosition);

    float orbitAngle = computeOrbitAngle(currentSeconds);
    model = glm::rotate(model, orbitAngle, orbitAxis);

    model = glm::translate(model, initialPosition);

    float rotationAngle = computeRotationAngle(currentSeconds); // in radians
    model = glm::rotate(model, rotationAngle, rotationAxis);

    return model;
}

glm::vec3 Moon::getCurrentPosition(long long currentSeconds) {
    glm::mat4 model(1.0);

    glm::vec3 currentEarthPosition = earth->getCurrentPosition(currentSeconds);
    model = glm::translate(model, currentEarthPosition);

    float orbitAngle = computeOrbitAngle(currentSeconds);
    model = glm::rotate(model, orbitAngle, orbitAxis);

    model = glm::translate(model, initialPosition);

    return glm::vec3(model * glm::vec4(initialPosition, 1.0));
}

float Moon::computeRotationAngle(long long currentSeconds) {
    currentSeconds = currentSeconds % rotationPeriod;
    float angle = ((float)currentSeconds / (float)rotationPeriod) * 2 * glm::pi<float>();
    return angle;
}

float Moon::computeOrbitAngle(long long currentSeconds) {
    currentSeconds = currentSeconds % orbitPeriod;
    float angle = ((float)currentSeconds / (float)orbitPeriod) * 2 * glm::pi<float>();
    return angle;
}

float Moon::getRadius() {
    return radius;
}

void Moon::render(const glm::mat4 *baseModel, const glm::mat4 *viewMatrix, long long currentSeconds) {
    glm::mat4 model = (*baseModel) * this->getModelByTime(currentSeconds);
    shader->sendModelUniform(model);
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(*viewMatrix * model));
    shader->sendNormalMatrixUniform(normalMatrix);
    object->Draw(*shader);
}

void Moon::renderDepthMap(const glm::mat4 *baseModel,
                          const glm::mat4 *viewMatrix,
                          long long currentSeconds,
                          gps::GeometryShader *geometryShader) {
    glm::mat4 model = (*baseModel) * this->getModelByTime(currentSeconds);
    geometryShader->setMat4("modelMatrix", model);
    object->Draw(*geometryShader);
}
} // namespace gps
