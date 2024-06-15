#version 330 core

flat in vec3 color2;
out vec4 FragColor;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float random2(vec2 st) {
    return fract(sin(dot(st.xy, vec2(52.9818,78.233))) * 758.3123);
}

float random3(vec2 st) {
    return fract(sin(dot(st.xy, vec2(10.9898,78.233))) * 438.5453123);
}

void main() {
    FragColor = vec4(color2, 1.0); 
    //FragColor = vec4(random(color.xy), random2(color.xy), random3(color.xy), 1.0); 
}