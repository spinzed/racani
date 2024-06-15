#version 330 core

uniform mat4 mMatrix;

uniform vec3 cameraPos;

uniform float lightNum;
uniform vec3[10]lightPosition;
uniform vec3[10]lightIntensity;
uniform vec3[10]lightColor;

uniform vec3 colorAmbient;
uniform vec3 colorDiffuse;
uniform vec3 colorSpecular;
uniform float shininess;

in vec3 color;
in vec3 normal;

out vec4 FragColor;

#define LIGHT_MAX_RANGE 100

void main(){
    //vec3 light=colorAmbient;
    FragColor=vec4(color,1.);
    //FragColor = vec4(random(color.xy), random2(color.xy), random3(color.xy), 1.0);
}