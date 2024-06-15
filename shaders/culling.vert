#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;

uniform mat4 mMatrix;
uniform mat4 pvMatrix;

flat out vec3 color;

void main() {
	color = aCol;
    gl_Position = pvMatrix * mMatrix * vec4(aPos, 1.0);
    //gl_Position = vec4(aPos, 1.0);
}