#version 330 core

in vec4 FragPos;
out vec4 FragColor;

uniform vec3 lightPos;
//uniform float far_plane;

void main() {
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);
    float farPlane = 10.0f;
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / farPlane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}
