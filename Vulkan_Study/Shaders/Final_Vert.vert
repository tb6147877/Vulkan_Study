#version 450

layout (location=0) out vec2 outUV;
layout (location=1) out flat int instanceID;
// shader taken from https://github.com/SaschaWillems/Vulkan/blob/master/data/shaders/glsl/deferred/deferred.vert
void main(){
    outUV=vec2((gl_VertexIndex<<1)&2,gl_VertexIndex&2);
    gl_Position=vec4(outUV*2.0f-1.0f,0.0f,1.0f);
    instanceID=gl_InstanceIndex;
}