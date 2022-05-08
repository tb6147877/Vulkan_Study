#version 450

layout (binding = 1) uniform sampler2D samplerPosition;
layout (binding = 2) uniform sampler2D samplerNormal;
layout (binding = 3) uniform sampler2D samplerAlbedo;

struct PointLight
{
    vec4 pos;
    vec4 color_radius;
};

layout (binding=4) uniform UniformBufferObjectFrag{
    PointLight[1] lights;
    vec4 viewPos;
    vec4 gap;
}ubo;

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

const vec3 ambient = vec3(0.2f, 0.2f, 0.2f);

#define NEAR 0.1f
#define FAR 40.0f

// compute linear depth, taken from:
// https://learnopengl.com/Advanced-OpenGL/Depth-testing
// (formula has been factorised)
float linZ (float z, float n, float f) {
	return n / (f - f*z + n*z);
}

void main(){
    outColor=texture(samplerPosition,inUV);
}