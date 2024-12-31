#version 330 core

layout(location=0)in vec3 aPos;
layout(location=1)in vec3 aCol;
layout(location=2)in vec3 aNorm;
layout(location=3)in vec2 aTex;

uniform mat4 mMatrix;
uniform mat4 pvMatrix;
uniform mat4 lightSpaceMatrix;

out vec3 color;
out vec3 normal;
out vec3 position;
out vec4 positionLightSpace;
out vec2 TexCoords;

void main(){
    color=aCol;
    mat3 normalMatrix = transpose(inverse(mat3(mMatrix)));
    normal = normalize(normalMatrix * aNorm);
    //normal = normalize(aNorm);
    //normal = normalize(vec3(mMatrix * vec4(aNorm, 1.0f)));
    TexCoords = aTex;
    vec4 pos = mMatrix*vec4(aPos,1.);
    position = pos.xyz;
    positionLightSpace = lightSpaceMatrix * pos;
    gl_Position=pvMatrix*pos;
}