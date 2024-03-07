#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat3 normalMatrix;

struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  
uniform PointLight sunLight;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

uniform samplerCube depthMap;
uniform float farPlane;
float shadow = 0.0;

const int shininess = 32;

vec3 gridSamplingDisk[20] = vec3[] (
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

void computePointShadow() {
    vec3 fragPos = (modelMatrix * vec4(fPosition, 1.0)).xyz;
	vec3 fragToLight = fragPos - sunLight.position;
    
    float currentDepth = length(fragToLight);

    float bias = 3;
    int samples = 20;
    vec3 viewPosition = (inverse(viewMatrix) * vec4(0, 0, 0, 1)).xyz;
    float viewDistance = length(viewPosition - fragPos); // (0, 0, 0) stands for the viewpos (in eye space)
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;

    for(int i = 0; i < samples; ++i) {
        float closestDepth = texture(depthMap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= farPlane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth) {
             shadow += 1.0;
        }
    }
    shadow /= float(samples);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDirection) {
    vec3 lightPosition = (viewMatrix * vec4(light.position, 1.0)).xyz;
    vec3 lightDirection = normalize(lightPosition - fragPos);
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);
    float distance    = length(lightPosition - fragPos);
    float attenuation =  1 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    
    vec3 ambient  = light.ambient  * vec3(texture(diffuseTexture, fTexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(diffuseTexture, fTexCoords));
    vec3 specular = light.specular * spec * vec3(texture(specularTexture, fTexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + (1.0 - shadow) * diffuse + (1.0 - shadow) * specular);
} 

void main() {
    vec4 fPosEye = viewMatrix * modelMatrix * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 viewDirection = normalize(- fPosEye.xyz);

    computePointShadow();
    vec3 result = CalcPointLight(sunLight, normalEye, fPosEye.xyz, viewDirection);

    fColor = vec4(min(result, 1.0), 1.0f);
}
