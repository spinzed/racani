#version 330 core

layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aCol;
layout(location=2)in vec3 aNorm;

uniform mat4 mMatrix;
uniform mat4 pvMatrix;

out vec3 fcolor;

void main(){
    fcolor=aCol;
    vec4 n=normalize(vec4(aNorm, 1));
    gl_Position=mMatrix*vec4(aPos,1.);
}