#version 330 core

in vec4 FragPos;

uniform vec3 lightPosition;
uniform float farPlane;

void main() {
    float lightDistance = length(FragPos.xyz - lightPosition);

    lightDistance = lightDistance / farPlane;
    
    gl_FragDepth = lightDistance;
}  
