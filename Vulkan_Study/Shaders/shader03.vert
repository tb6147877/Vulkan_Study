#version 450

//当同时有多个descriptor set时，需要使用layout(set = 0, binding = 0) 来引用descriptor set
layout(binding=0) uniform UniformBufferObject{
    mat4 model;
    mat4 view;
    mat4 proj;
}ubo;

layout(location=0)in vec2 inPosition;//vertex arribute
layout(location=1)in vec3 inColor;//vertex arribute

layout(location = 0)out vec3 fragColor;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}
