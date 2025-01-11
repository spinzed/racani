#version 330 core

layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aCol;
layout(location=2)in vec3 aNorm;
layout(location=3)in vec2 aTex;

uniform mat4 mMatrix;
uniform mat4 pvMatrix;
uniform float pointSize;

out vec3 color;

void main() {
    color=aCol;
    gl_Position=pvMatrix*mMatrix*vec4(aPos,1.);
    gl_PointSize = pointSize;
}
