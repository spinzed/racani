#version 150 core

layout(triangles)in;
layout(line_strip,max_vertices=3)out;

uniform vec3 cameraPos;
uniform vec3 cameraDirection;
uniform mat4 pvMatrix;

flat in vec3 color[3];

flat out vec3 color2;

// OVI SHADER VALJA JA MSN
void main(){
    vec3 v0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 v1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
    vec3 v2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;

    vec3 c=(v0+v1+v2)/3;
    vec3 ray=cameraPos -c;
    vec3 brid1=v1-v0;
    vec3 brid2=v2-v0;
    vec3 normal=normalize(cross(brid1,brid2));
    
    if(dot(normal,ray)<0) return;

    for (int i = 0; i < 3; i++) {
        gl_Position=pvMatrix*gl_in[i].gl_Position;
        color2=color[i];
        EmitVertex();
    }

    EndPrimitive();
}