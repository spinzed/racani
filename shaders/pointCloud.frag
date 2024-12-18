#version 330 core

in vec3 color;
in vec3 normal;

out vec4 FragColor;

void main(){
    //vec3 light=colorAmbient;
    FragColor=vec4(color,1.);
    //FragColor = vec4(random(color.xy), random2(color.xy), random3(color.xy), 1.0);
}