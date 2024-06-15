#version 330 core

layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aCol;
layout(location=2)in vec3 aNorm;

uniform mat4 mMatrix;
uniform mat4 pvMatrix;

out vec3 fcolor;
out vec3 fpointNormal;

void main(){
    fcolor=aCol;
    fpointNormal = normalize(aNorm);
    gl_Position=mMatrix*vec4(aPos,1.);
}