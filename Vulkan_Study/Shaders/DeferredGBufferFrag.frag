#version 450

layout (binding=1) uniform sampler2D diffTex;

//input from previous shader stage
layout(location=0) in vec3 fragPos;
layout(location=1) in vec3 fragNormal;
layout(location=2) in vec2 fragTexCoord;

layout(location=0) out vec4 outPosition;
layout(location=1) out vec4 outNormal;
layout(location=2) out vec4 outAlbedo;

const float far=40.0f;
const float near=0.1;

float linearize_Z(float z , float zNear , float zFar){
    return (2 * zNear * zFar) / (zFar + zNear - (z * 2.0f  - 1.0f) * (zFar -zNear)) ;
}

void main(){
    outPosition=vec4(fragPos,linearize_Z(gl_FragCoord.z,near,far)/far);
    outNormal=vec4(fragNormal,1.0f);
    outAlbedo=texture(diffTex,fragTexCoord);
}