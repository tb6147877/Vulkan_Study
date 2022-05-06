#version 450


layout (binding=1) uniform sampler2D diffTex;

struct PointLight
{
    vec4 pos;
    vec4 color_radius;
};

layout (binding=2) uniform UniformBufferObjectFrag{
    PointLight[1] lights;
    vec4 viewPos;
    vec4 gap;
}ubo;

//input from previous shader stage
layout(location=0) in vec3 fragPos;
layout(location=1) in vec3 fragNormal;
layout(location=2) in vec2 fragTexCoord;

//output
layout(location=0) out vec4 outColor;

#define POINT_LIGHT_NUM 1
vec3 calculatePointLight(PointLight light, vec3 viewDir, vec3 normal, vec3 fragPos, vec3 diffuseColor);

void main(){
    vec3 result = vec3(0.0);
    vec3 viewDir=normalize(ubo.viewPos.xyz-fragPos);
    vec3 normal=normalize(fragNormal);
    vec3 diffuseColor=texture(diffTex,fragTexCoord).xyz;
    for(int i=0;i<POINT_LIGHT_NUM;i++){
        result += calculatePointLight(ubo.lights[i],viewDir,normal,fragPos,diffuseColor);
    }
    
    outColor=vec4(result,1.0);
}

vec3 calculatePointLight(PointLight light, vec3 viewDir, vec3 normal, vec3 fragPos, vec3 diffuseColor)
{
    float dis=length(light.pos.xyz-fragPos);
    float atten=1.0-clamp(dis/light.color_radius.w,0.0,1.0);
    
    vec3 result=vec3(0.0);
    vec3 lightDir=normalize(light.pos.xyz-fragPos);
    float diff=max(dot(normal,lightDir),0.0);
    vec3 halfDir=normalize(viewDir+lightDir);
    float spec=pow(max(dot(halfDir,normal),0.0),32.0);

    vec3 ambient=light.color_radius.xyz*diffuseColor*0.1;
    vec3 diffuse=light.color_radius.xyz*diffuseColor*diff;
    vec3 specular=light.color_radius.xyz*0.4*spec;
    
    ambient*=atten;
    diffuse*=atten;
    specular*=atten;
    
    result=ambient+diffuse+specular;
    return result;
}

