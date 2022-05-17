#version 450

layout (binding = 5) uniform sampler2D outputTex;

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main(){
    outColor=texture(outputTex,inUV);
}

