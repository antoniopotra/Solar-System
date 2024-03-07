#include "Earth.hpp"

namespace gps {

Earth::Earth(glm::vec3 initialPosition, long long rotationPeriod, glm::vec3 rotationAxis, long long orbitPeriod, glm::vec3 orbitAxis, float radius, std::string filename, gps::Shader *shader) {
    this->initialPosition = initialPosition;
    this->rotationPeriod = rotationPeriod;
    this->rotationAxis = rotationAxis;
    this->orbitPeriod = orbitPeriod;
    this->orbitAxis = orbitAxis;
    this->radius = radius;
    this->object = new gps::Model3D();
    this->object->LoadModel(filename);
    this->shader = shader;
}

glm::mat4 Earth::getModelByTime(long long currentSeconds) {
    glm::mat4 model(1.0);

    float orbitAngle = computeOrbitAngle(currentSeconds);
    model = glm::rotate(model, orbitAngle, orbitAxis);

    model = glm::translate(model, initialPosition);

    float rotation_angle = computeRotationAngle(currentSeconds);
    model = glm::rotate(model, rotation_angle, rotationAxis);

    return model;
}

glm::vec3 Earth::getCurrentPosition(long long currentSeconds) {
    glm::mat4 model(1.0);

    float orbitAngle = computeOrbitAngle(currentSeconds);
    model = glm::rotate(model, orbitAngle, orbitAxis);

    return glm::vec3(model * glm::vec4(initialPosition, 1.0));
}

float Earth::computeRotationAngle(long long currentSeconds) {
    currentSeconds = currentSeconds % rotationPeriod;
    float angle = ((float)currentSeconds / (float)rotationPeriod) * 2 * glm::pi<float>();
    return angle;
}

float Earth::computeOrbitAngle(long long currentSeconds) {
    currentSeconds = currentSeconds % orbitPeriod;
    float angle = ((float)currentSeconds / (float)orbitPeriod) * 2 * glm::pi<float>();
    return angle;
}

float Earth::getRadius() {
    return radius;
}

void Earth::render(const glm::mat4 *baseModel, const glm::mat4 *viewMatrix, long long currentSeconds) {
    glm::mat4 model = (*baseModel) * this->getModelByTime(currentSeconds);
    shader->sendModelUniform(model);
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(*viewMatrix * model));
    shader->sendNormalMatrixUniform(normalMatrix);
    object->Draw(*shader);
}

void Earth::renderDepthMap(const glm::mat4 *baseModel,
                           const glm::mat4 *viewMatrix,
                           long long currentSeconds,
                           gps::GeometryShader *geometryShader) {
    glm::mat4 model = (*baseModel) * this->getModelByTime(currentSeconds);
    geometryShader->setMat4("modelMatrix", model);
    object->Draw(*geometryShader);
}
} // namespace gps
