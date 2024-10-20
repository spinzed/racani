#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 mMatrix;

void main() {
    gl_Position = mMatrix * vec4(aPos, 1.0);
}  
