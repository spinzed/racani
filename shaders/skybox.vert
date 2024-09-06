#version 330 core
layout(location=0)in vec3 aPos;

uniform mat4 pvCenterMatrix;
out vec3 TexCoords;

void main(){
    TexCoords = aPos;
    gl_Position = pvCenterMatrix * vec4(aPos, 1.0);
}
