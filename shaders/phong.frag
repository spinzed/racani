#version 330 core

uniform mat4 mMatrix;

uniform vec3 cameraPos;

uniform int lightNum;
uniform vec3[10]lightPosition;
uniform vec3[10]lightIntensity;
uniform vec3[10]lightColor;

uniform vec3 colorAmbient;
uniform vec3 colorDiffuse;
uniform vec3 colorSpecular;
uniform float shininess;

uniform bool hasTextures;
uniform sampler2D texture1;
uniform bool hasShadowMap;
uniform sampler2D shadowMap;

in vec3 color;
in vec3 normal;
in vec3 position;
in vec4 positionLightSpace;
in vec2 TexCoords;

in vec3 FragPos;

out vec4 FragColor;

#define LIGHT_MAX_RANGE 20

float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = 0.0005;
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    //return currentDepth - bias > closestDepth ? 1.0 : 0.0;
    return shadow;
}

vec3 calculateLight(vec3 lightPos,vec3 lightIntensity,vec3 lightColor){
    //vec3 cameraPos = vec3(0.0, 2.0, 1.0);
    
    //float diffuseStrength = dot(lightPos, normal);
    float diffuseStrength=max(0.,dot(lightPos,normal));
    
    vec3 reflected=normalize(reflect(-lightPos,normal));
    float specularBase=max(0.,dot(normalize(cameraPos),reflected));
    float specularStrength=pow(specularBase,32);

    float d = distance(lightPos, position);
    float i = max((LIGHT_MAX_RANGE - d) / LIGHT_MAX_RANGE, 0.0f);
    
    vec3 lighting=lightColor*(diffuseStrength+specularStrength)*lightIntensity * i;
    
    return lighting;
}

void main(){
    //vec3 light=colorAmbient;
    vec3 light=vec3(.2,.2,.2);
    vec3 colorToShade; 
    if (hasTextures) {
        colorToShade = texture(texture1, TexCoords).xyz;
    } else {
        colorToShade = color;
    }

    float shadow = hasShadowMap ? ShadowCalculation(positionLightSpace) : 0;

    for(int i=0;i<1&&i<lightNum;i++){
        vec4 lp=vec4(lightPosition[i],1);
        light+= (1.0-shadow) * calculateLight(lp.xyz,lightIntensity[i],lightColor[i]);
    }

    FragColor=vec4(light*colorToShade,1.);
}