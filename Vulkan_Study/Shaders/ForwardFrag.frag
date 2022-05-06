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
};

//input from previous shader stage
layout(location=0) in vec3 fragPos;
layout(location=1) in vec3 fragNormal;
layout(location=2) in vec2 fragTexCoord;

//output
layout(location=0) out vec4 outColor;

void main(){
    //outColor=vec4(1.0,1.0,1.0,1.0);
    outColor=texture(diffTex,fragTexCoord);
}

