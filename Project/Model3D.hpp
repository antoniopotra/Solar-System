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

#include "Mesh.hpp"
#include "stb_image.hpp"
#include "tiny_obj_loader.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace gps {

class Model3D {

public:
    ~Model3D();

    void LoadModel(std::string fileName);

    void LoadModel(std::string fileName, std::string basePath);

    void Draw(gps::Shader shaderProgram);

private:
    // Component meshes - group of objects
    std::vector<gps::Mesh> meshes;
    // Associated textures
    std::vector<gps::Texture> loadedTextures;

    // Does the parsing of the .obj file and fills in the data structure
    void ReadOBJ(std::string fileName, std::string basePath);

    // Retrieves a texture associated with the object - by its name and type
    gps::Texture LoadTexture(std::string path, std::string type);

    // Reads the pixel data from an image file and loads it into the video memory
    GLuint ReadTextureFromFile(const char *file_name);
};
} // namespace gps
