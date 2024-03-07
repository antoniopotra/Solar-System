#include "Shader.hpp"

namespace gps {
std::string Shader::readShaderFile(std::string filename) {
    std::ifstream shaderFile;
    std::string shaderString;

    shaderFile.open(filename.c_str());
    std::stringstream shaderStringStream;
    shaderStringStream << shaderFile.rdbuf();
    shaderFile.close();

    shaderString = shaderStringStream.str();
    return shaderString;
}

void Shader::shaderCompileLog(GLuint shaderId) {
    GLint success;
    GLchar infoLog[512];

    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
        std::cout << "Shader compilation error\n"
                  << infoLog << std::endl;
    }
}

void Shader::shaderLinkLog(GLuint shaderProgramId) {
    GLint success;
    GLchar infoLog[512];

    glGetProgramiv(shaderProgramId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "Shader linking error\n"
                  << infoLog << std::endl;
    }
}

void Shader::loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName) {
    std::string v = readShaderFile(vertexShaderFileName);
    const GLchar *vertexShaderString = v.c_str();
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderString, NULL);
    glCompileShader(vertexShader);
    shaderCompileLog(vertexShader);

    std::string f = readShaderFile(fragmentShaderFileName);
    const GLchar *fragmentShaderString = f.c_str();
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderString, NULL);
    glCompileShader(fragmentShader);
    shaderCompileLog(fragmentShader);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    shaderLinkLog(shaderProgram);
}

void Shader::useShaderProgram() {
    glUseProgram(shaderProgram);
}

void Shader::sendModelUniform(glm::mat4 modelMatrix) {
    useShaderProgram();
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));
}

void Shader::sendViewUniform(glm::mat4 viewMatrix) {
    useShaderProgram();
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));
}

void Shader::sendProjectionUniform(glm::mat4 projectionMatrix) {
    useShaderProgram();
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
}

void Shader::sendNormalMatrixUniform(glm::mat3 normalMatrix) {
    useShaderProgram();
    glUniformMatrix3fv(normalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void Shader::sendDirectionalLightUniform(gps::DirectionalLight light) {
    useShaderProgram();
    glUniform3fv(glGetUniformLocation(shaderProgram, "light.direction"), 1, glm::value_ptr(light.direction));
    glUniform3fv(glGetUniformLocation(shaderProgram, "light.color"), 1, glm::value_ptr(light.color));
    glUniform1f(glGetUniformLocation(shaderProgram, "light.ambientStrength"), light.ambientStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "light.diffuseStrength"), light.diffuseStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "light.specularStrength"), light.specularStrength);
}

void Shader::sendSunLightUniform(gps::PointLight sunLight) {
    useShaderProgram();
    glUniform3fv(glGetUniformLocation(shaderProgram, "sunLight.position"), 1, glm::value_ptr(sunLight.position));
    glUniform1f(glGetUniformLocation(shaderProgram, "sunLight.constant"), sunLight.constant);
    glUniform1f(glGetUniformLocation(shaderProgram, "sunLight.linear"), sunLight.linear);
    glUniform1f(glGetUniformLocation(shaderProgram, "sunLight.quadratic"), sunLight.quadratic);
    glUniform3fv(glGetUniformLocation(shaderProgram, "sunLight.ambient"), 1, glm::value_ptr(sunLight.ambient));
    glUniform3fv(glGetUniformLocation(shaderProgram, "sunLight.diffuse"), 1, glm::value_ptr(sunLight.diffuse));
    glUniform3fv(glGetUniformLocation(shaderProgram, "sunLight.specular"), 1, glm::value_ptr(sunLight.specular));
}

void Shader::initUniforms() {
    useShaderProgram();
    modelLocation = glGetUniformLocation(shaderProgram, "modelMatrix");
    viewLocation = glGetUniformLocation(shaderProgram, "viewMatrix");
    normalMatrixLocation = glGetUniformLocation(shaderProgram, "normalMatrix");
    projectionLocation = glGetUniformLocation(shaderProgram, "projectionMatrix");
}

void Shader::init(std::string vertexShaderPath, std::string fragmentShaderPath) {
    loadShader(vertexShaderPath, fragmentShaderPath);
    initUniforms();
}
} // namespace gps
