#version 150 core

layout(triangles)in;
layout(triangle_strip,max_vertices=3)out;

uniform vec3 cameraPos;
uniform vec3 cameraDirection;
uniform mat4 pvMatrix;

uniform float lightNum;
uniform vec3[10]lightPosition;
uniform vec3[10]lightIntensity;
uniform vec3[10]lightColor;

uniform vec3 colorAmbient;
uniform vec3 colorDiffuse;
uniform vec3 colorSpecular;
uniform float shininess;

in vec3 fcolor[3];
in vec3 fpointNormal[3];

out vec3 color;

#define LIGHT_MAX_RANGE 10.0// dl

vec3 calculateLight(vec3 normal, vec3 lightPos,vec3 lightIntensity,vec3 lightColor){
    float diffuseStrength=max(0.,dot(lightPos,normal));
    
    vec3 reflected=normalize(reflect(-lightPos,normal));
    float specularBase=max(0.,dot(normalize(cameraPos),reflected));
    float specularStrength=pow(specularBase,32);
    
    //vec4 pvLightPos = pvMatrix*vec4(lightPos,1);
    //float d=distance(lightPos.xyz,position);
    //float i=max((LIGHT_MAX_RANGE-d)/LIGHT_MAX_RANGE,0.f);
    
    vec3 lighting=lightColor*lightIntensity*(diffuseStrength*colorDiffuse+specularStrength*colorSpecular);
    
    return lighting;
}


// OVI SHADER VALJA JA MSN
void main(){
    vec3 v0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
    vec3 v1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
    vec3 v2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;

    vec3 c=(v0+v1+v2)/3;
    //position = c;
    vec3 ray=cameraPos -c;
    vec3 brid1=v1-v0;
    vec3 brid2=v2-v0;
    vec3 normal=normalize(cross(brid1,brid2));
    
    //if(dot(normal,ray)<0) return;

    for (int i = 0; i < 3; i++) {
        gl_Position=pvMatrix* gl_in[i].gl_Position;

        vec3 light=colorAmbient;
        for(int j=0;j<lightNum;j++){
            light+=calculateLight(fpointNormal[i], lightPosition[j],lightIntensity[j],lightColor[j]); // lightColor i lightIntensity su Ip
        }
        color = fcolor[i] * light;
    
        EmitVertex();
    }

    EndPrimitive();
}