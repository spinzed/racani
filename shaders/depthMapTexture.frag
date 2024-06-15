#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main(){
    //FragColor=texture(texture1,TexCoord);
    float depthValue = texture(texture1, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}