#include "Sun.hpp"

namespace gps {

Sun::Sun(glm::vec3 initialPosition, long long rotationPeriod, glm::vec3 rotationAxis, float radius, std::string filename, gps::Shader *shader) {
    this->initialPosition = initialPosition;
    this->rotationPeriod = rotationPeriod;
    this->rotationAxis = rotationAxis;
    this->radius = radius;
    this->object = new gps::Model3D();
    this->object->LoadModel(filename);
    this->shader = shader;
}

glm::mat4 Sun::getModelByTime(long long currentSeconds) {
    glm::mat4 model(1.0);

    model = glm::translate(model, initialPosition);

    float rotationAngle = computeRotationAngle(currentSeconds); // in radians
    model = glm::rotate(model, rotationAngle, rotationAxis);

    return model;
}

glm::vec3 Sun::getCurrentPosition(long long currentSeconds) {
    return initialPosition;
}

float Sun::computeRotationAngle(long long currentSeconds) {
    currentSeconds %= rotationPeriod;
    float angle = ((float)currentSeconds / (float)rotationPeriod) * 2 * glm::pi<float>();
    return angle;
}

float Sun::getRadius() {
    return radius;
}

void Sun::render(const glm::mat4 *baseModel, const glm::mat4 *viewMatrix, long long currentSeconds) {
    glm::mat4 model = (*baseModel) * this->getModelByTime(currentSeconds);
    shader->sendModelUniform(model);
    glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(*viewMatrix * model));
    shader->sendNormalMatrixUniform(normalMatrix);
    object->Draw(*shader);
}

void Sun::renderDepthMap(const glm::mat4 *baseModel,
                         const glm::mat4 *viewMatrix,
                         long long currentSeconds,
                         gps::GeometryShader *geometryShader) {
    glm::mat4 model = (*baseModel) * this->getModelByTime(currentSeconds);
    geometryShader->setMat4("modelMatrix", model);
    object->Draw(*geometryShader);
}

} // namespace gps
