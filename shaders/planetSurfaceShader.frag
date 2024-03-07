#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float ambientStrength;
    float diffuseStrength;
    float specularStrength;
};
uniform DirectionalLight light;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

const int shininess = 32;

vec3 computeLight(vec3 normal, vec3 viewDirection) {
    vec3 lightDirection = vec3(normalize(viewMatrix * vec4(light.direction, 0.0f)));
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);
    vec3 ambient  = light.color  * vec3(texture(diffuseTexture, fTexCoords)) * light.ambientStrength;
    vec3 diffuse  = light.color  * diff * vec3(texture(diffuseTexture, fTexCoords)) * light.diffuseStrength;
    vec3 specular = light.color * spec * vec3(texture(specularTexture, fTexCoords)) * light.specularStrength;
    return (ambient + diffuse + specular);
}

float computeFog(vec3 fPosEye) {
    float fogDensity = 0.002f;
    float fragmentDistance = length(fPosEye);
    float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
    return clamp(fogFactor, 0.0f, 1.0f);
}

void main() {
    vec4 fPosEye = viewMatrix * modelMatrix * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 viewDirection = normalize(-fPosEye.xyz);
    vec3 lightResult = computeLight(normalEye, viewDirection);
    
    float fogFactor = computeFog(fPosEye.xyz);
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    fColor = fogColor * (1 - fogFactor) + vec4(lightResult * fogFactor, 1.0);
}
