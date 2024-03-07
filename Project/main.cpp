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

#include "Camera.hpp"
#include "GeometryShader.h"
#include "Light.h"
#include "Model3D.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "SolarSystem.hpp"

#include <iostream>

GLFWwindow *window;
int windowWidth = 800, windowHeight = 600;

glm::mat4 modelMatrix;
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;
glm::mat3 normalMatrix;

gps::Camera spaceCamera(glm::vec3(-500.0f, 0.0f, 0.0f),
                        glm::vec3(2000.0f, 0.0f, 0.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f));
gps::Camera planetCamera(glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(28.0f, 0.0f, 0.0f),
                         glm::vec3(0.0f, 1.0f, 0.0f));
GLfloat cameraMoveSpeed = 300.0f;
GLfloat cameraRotationSpeed = 20.0f;
bool cameraAnimationButtonPressed = false;
float currentCameraAngle = 0;

GLboolean pressedKeys[1024];

GLfloat angle;

gps::Shader planetShader;
gps::Shader skyboxShader;
gps::Shader sunShader;
gps::GeometryShader sunDepthShader;
gps::Shader planetSurfaceShader;

gps::SolarSystem solarSystem;

float deltaTime = 0;
double lastTime = glfwGetTime();
double currentTime = 0;
double simulationTime = 0;
bool stopped = false;
bool enterPressed = false;

std::vector<const GLchar *> skyboxFaces;
gps::Skybox skybox;

gps::PointLight sunLight = {
    glm::vec3(0, 0, 0),
    1.0,
    0.000014,
    0.000000007,
    glm::vec3(0.1, 0.05, 0.0),
    glm::vec3(1.0, 0.95, 0.75),
    glm::vec3(1.0, 0.95, 0.75)};
gps::DirectionalLight planetSurfaceLight = {
    glm::vec3(-1.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    0.6,
    0.8,
    1.0};

const double ANIMATION_TIME = 3600 * 24 * 3.65;

unsigned int depthCubemap;
const unsigned int SHADOW_WIDTH = 10240, SHADOW_HEIGHT = 10240;
unsigned int depthMapFbo;
std::vector<glm::mat4> shadowTransforms;
const float DEPTH_FAR_PLANE = 20000;

float lastMouseX = 400, lastMouseY = 300;
const float MOUSE_SENSITIVITY = 0.1f;
bool mouseEnabled = false;
bool mouseButtonPressed = false;

bool changeDisplayMode = false;
int currentDisplayMode = 0;

bool surfaceSceneOn = false;
bool landingDetectionOn = true;
gps::Model3D planetSurface;
glm::mat4 planetModelMatrix(1.0);

void createWindow() {
    if (!glfwInit()) {
        throw std::runtime_error("Could not start GLFW3!");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    glfwWindowHint(GLFW_SAMPLES, 4);

    window = glfwCreateWindow(windowWidth, windowHeight, "Solar System", NULL, NULL);
    if (!window) {
        throw std::runtime_error("Could not create GLFW3 window!");
    }

    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte *version = glGetString(GL_VERSION);   // version as a string
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version: " << version << std::endl;

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
}

void destroyWindow() {
    if (window)
        glfwDestroyWindow(window);
    glfwTerminate();
}

void setupShadowTransforms() {
    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    float near = 1.0f;
    float far = DEPTH_FAR_PLANE;
    glm::mat4 shadowProjection = glm::perspective(glm::radians(90.0f), aspect, near, far);

    shadowTransforms.clear();
    shadowTransforms.push_back(shadowProjection *
                               glm::lookAt(sunLight.position, sunLight.position + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProjection *
                               glm::lookAt(sunLight.position, sunLight.position + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProjection *
                               glm::lookAt(sunLight.position, sunLight.position + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowProjection *
                               glm::lookAt(sunLight.position, sunLight.position + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
    shadowTransforms.push_back(shadowProjection *
                               glm::lookAt(sunLight.position, sunLight.position + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
    shadowTransforms.push_back(shadowProjection *
                               glm::lookAt(sunLight.position, sunLight.position + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
}

void initShadows() {
    glGenFramebuffers(1, &depthMapFbo);

    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    setupShadowTransforms();
}

void updateDelta() {
    lastTime = currentTime;
    currentTime = glfwGetTime();
    deltaTime = currentTime - lastTime;
    if (!stopped) {
        simulationTime += deltaTime;
    }
}

GLenum glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void updateProjection() {
    projectionMatrix = glm::perspective(glm::radians(55.0f), (float)windowWidth / (float)windowHeight, 0.1f, DEPTH_FAR_PLANE);
    planetShader.sendProjectionUniform(projectionMatrix);
    skyboxShader.sendProjectionUniform(projectionMatrix);
    sunShader.sendProjectionUniform(projectionMatrix);
    planetSurfaceShader.sendProjectionUniform(projectionMatrix);
}

void windowResizeCallback(GLFWwindow *window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glViewport(0, 0, windowWidth, windowHeight);
    updateProjection();
}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    float xoffset = xpos - lastMouseX;
    float yoffset = ypos - lastMouseY;
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (!mouseEnabled) {
        return;
    }
    if (!surfaceSceneOn) {
        xoffset *= MOUSE_SENSITIVITY;
        yoffset *= MOUSE_SENSITIVITY;
        spaceCamera.rotate(cameraRotationSpeed * yoffset * deltaTime, cameraRotationSpeed * xoffset * deltaTime);
    } else {
        xoffset *= MOUSE_SENSITIVITY;
        yoffset *= MOUSE_SENSITIVITY;
        planetCamera.rotate(cameraRotationSpeed * yoffset * deltaTime, cameraRotationSpeed * xoffset * deltaTime);
    }
}

void processMovement() {
    if (!surfaceSceneOn) {
        if (pressedKeys[GLFW_KEY_W]) {
            spaceCamera.move(gps::MOVE_FORWARD, cameraMoveSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_S]) {
            spaceCamera.move(gps::MOVE_BACKWARD, cameraMoveSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_A]) {
            spaceCamera.move(gps::MOVE_LEFT, cameraMoveSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_D]) {
            spaceCamera.move(gps::MOVE_RIGHT, cameraMoveSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_Q]) {
            angle -= 1.0f * deltaTime;
            modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        }
        if (pressedKeys[GLFW_KEY_E]) {
            angle += 1.0f * deltaTime;
            modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        }
        if (pressedKeys[GLFW_KEY_Y]) {
            spaceCamera.rotate(0, -cameraRotationSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_T]) {
            spaceCamera.rotate(0, cameraRotationSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_P]) {
            spaceCamera.rotate(cameraRotationSpeed * deltaTime, 0);
        }
        if (pressedKeys[GLFW_KEY_L]) {
            spaceCamera.rotate(-cameraRotationSpeed * deltaTime, 0);
        }
        if (pressedKeys[GLFW_KEY_ENTER]) {
            if (!enterPressed) {
                stopped = !stopped;
                enterPressed = true;
            }
        } else {
            enterPressed = false;
        }
        if (pressedKeys[GLFW_KEY_V]) {
            if (!cameraAnimationButtonPressed) {
                spaceCamera.isAnimating = !spaceCamera.isAnimating;
                currentCameraAngle = 0;
                cameraAnimationButtonPressed = true;
            }
        } else {
            cameraAnimationButtonPressed = false;
        }
        if (spaceCamera.isAnimating) {
            currentCameraAngle += 0.3f;
            spaceCamera.animate(currentCameraAngle);
        }
    } else {
        if (pressedKeys[GLFW_KEY_W]) {
            planetCamera.move(gps::MOVE_FORWARD, cameraMoveSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_S]) {
            planetCamera.move(gps::MOVE_BACKWARD, cameraMoveSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_A]) {
            planetCamera.move(gps::MOVE_LEFT, cameraMoveSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_D]) {
            planetCamera.move(gps::MOVE_RIGHT, cameraMoveSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_Y]) {
            planetCamera.rotate(0, -cameraRotationSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_T]) {
            planetCamera.rotate(0, cameraRotationSpeed * deltaTime);
        }
        if (pressedKeys[GLFW_KEY_P]) {
            planetCamera.rotate(cameraRotationSpeed * deltaTime, 0);
        }
        if (pressedKeys[GLFW_KEY_L]) {
            planetCamera.rotate(-cameraRotationSpeed * deltaTime, 0);
        }
        if (pressedKeys[GLFW_KEY_UP]) {
            surfaceSceneOn = false;
        }
    }
    if (pressedKeys[GLFW_KEY_M]) {
        if (!mouseButtonPressed) {
            mouseEnabled = !mouseEnabled;
            mouseButtonPressed = true;
        }
    } else {
        mouseButtonPressed = false;
    }
    if (pressedKeys[GLFW_KEY_Z]) {
        if (!changeDisplayMode) {
            if (currentDisplayMode != 2) {
                currentDisplayMode++;
            } else {
                currentDisplayMode = 0;
            }
            if (currentDisplayMode == 0) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            } else if (currentDisplayMode == 1) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else if (currentDisplayMode == 2) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            }
            changeDisplayMode = true;
        }
    } else {
        changeDisplayMode = false;
    }
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetKeyCallback(window, keyboardCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initModels() {
    solarSystem.init(&planetShader, &sunShader);
    planetSurface.LoadModel("models/planet_surface/planet_surface.obj");
}

void initShaders() {
    planetShader.init("shaders/planetShader.vert", "shaders/planetShader.frag");
    skyboxShader.init("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    sunDepthShader.LoadShader("shaders/depthMapForSun.vert", "shaders/depthMapForSun.frag", "shaders/depthMapForSun.geo");
    sunShader.init("shaders/sunShader.vert", "shaders/sunShader.frag");
    planetSurfaceShader.init("shaders/planetSurfaceShader.vert", "shaders/planetSurfaceShader.frag");
}

void initSkyBox() {
    skyboxFaces.push_back("models/skybox/right.png");
    skyboxFaces.push_back("models/skybox/left.png");
    skyboxFaces.push_back("models/skybox/top.png");
    skyboxFaces.push_back("models/skybox/bottom.png");
    skyboxFaces.push_back("models/skybox/front.png");
    skyboxFaces.push_back("models/skybox/back.png");
    skybox.Load(skyboxFaces);
}

void initUniforms() {
    modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    viewMatrix = spaceCamera.getViewMatrix();
    planetShader.sendViewUniform(viewMatrix);
    skyboxShader.sendViewUniform(viewMatrix);
    sunShader.sendViewUniform(viewMatrix);

    normalMatrix = glm::mat3(glm::inverseTranspose(viewMatrix * modelMatrix));

    updateProjection();

    planetShader.sendSunLightUniform(sunLight);

    sunDepthShader.setFloat("farPlane", DEPTH_FAR_PLANE);
    for (unsigned int i = 0; i < 6; ++i) {
        sunDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
    }
    sunDepthShader.setVec3("lightPosition", sunLight.position);
    glUseProgram(planetShader.shaderProgram);
    glUniform1f(glGetUniformLocation(planetShader.shaderProgram, "farPlane"), DEPTH_FAR_PLANE);

    planetSurfaceShader.sendDirectionalLightUniform(planetSurfaceLight);

    planetSurfaceShader.sendViewUniform(planetCamera.getViewMatrix());

    planetSurfaceShader.sendModelUniform(planetModelMatrix);
}

void updateView() {
    if (!surfaceSceneOn) {
        viewMatrix = spaceCamera.getViewMatrix();
        planetShader.sendViewUniform(viewMatrix);
        sunShader.sendViewUniform(viewMatrix);
    } else {
        viewMatrix = planetCamera.getViewMatrix();
        planetSurfaceShader.sendViewUniform(viewMatrix);
    }
}

void renderScene() {
    if (!surfaceSceneOn) {
        setupShadowTransforms();
        
        updateView();
        
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        solarSystem.renderDepthMap(&modelMatrix, &viewMatrix, simulationTime * ANIMATION_TIME, &sunDepthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        planetShader.useShaderProgram();
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        glUniform1i(glGetUniformLocation(planetShader.shaderProgram, "depthMap"), 8);
        
        solarSystem.render(&modelMatrix, &viewMatrix, simulationTime * ANIMATION_TIME);
        
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        skybox.Draw(skyboxShader, viewMatrix, projectionMatrix);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    } else {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        updateView();

        planetSurfaceShader.useShaderProgram();

        glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(viewMatrix * planetModelMatrix));

        planetSurfaceShader.sendNormalMatrixUniform(normalMatrix);

        planetSurface.Draw(planetSurfaceShader);
    }
}

void detectLandings() {
    if (!surfaceSceneOn) {
        glm::vec3 cameraPosition = spaceCamera.getCameraPosition();

        bool landed = solarSystem.hasLandedOnEarth(simulationTime * ANIMATION_TIME, cameraPosition);

        if (landed) {
            if (landingDetectionOn) {
                surfaceSceneOn = true;
                landingDetectionOn = false;
            }
        } else {
            if (!landingDetectionOn) {
                landingDetectionOn = true;
            }
        }
    }
}

int main(int argc, const char *argv[]) {
    try {
        createWindow();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShadows();
    initShaders();
    initSkyBox();
    initUniforms();
    setWindowCallbacks();

    glCheckError();

    while (!glfwWindowShouldClose(window)) {
        updateDelta();
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(window);

        glCheckError();

        detectLandings();
    }

    destroyWindow();

    return EXIT_SUCCESS;
}
