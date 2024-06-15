#version 150 core

layout(triangles)in;
layout(triangle_strip,max_vertices=3)out;

flat in vec3 color[3];

flat out vec3 color2;

void main(){
    vec3 v0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 v1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
    vec3 v2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;

    // donekle valja
    //vec3 brid1=v1-v0;
    //vec3 brid2=v2-v0;
    //vec3 normala = normalize(cross(brid1, brid2));
    //if (dot(normala, vec3(0,0,1)) < 0) return;

    // skoro valja ako se ne dili s w, donekle valja ako
    //vec3 c = cross(v0,v1);
    //if(dot(v2,c)<0)return;

    // radi kad se gori ne podili s w, malo losije ako se podili
    float z_cross_product = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
    if (z_cross_product < 0) return;

    // ne valja
    //if (dot(v2.xy, v1.xy - v0.xy) < 0) return;

    for(int i=0;i<3;i++){
        gl_Position=gl_in[i].gl_Position;
        //gl_Position /= gl_Position.w; // jel triba?
        color2=color[i];
        EmitVertex();
    }
    EndPrimitive();
}