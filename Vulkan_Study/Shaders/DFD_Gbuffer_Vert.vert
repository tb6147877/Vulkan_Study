#version 450


//Uniform
layout (binding=0) uniform UniformBufferObjectVert{
    mat4 model;
    mat4 view;
    mat4 proj;
    
} ubo;

//inputs specified in the vertex buffer attributes
layout (location=0) in vec3 inPosition;
layout (location=1) in vec3 inNormal;
layout (location=2) in vec2 inTexCoord;

//output to next shader stage
layout (location=0) out vec3 fragPos;
layout (location=1) out vec3 fragNormal;
layout (location=2) out vec2 fragTexCoord;

void main(){
    gl_Position=ubo.proj*ubo.view*ubo.model*vec4(inPosition,1.0);
    
    fragPos=(ubo.model*vec4(inPosition,1.0)).xyz;
    fragNormal=mat3(transpose(inverse(ubo.model))) * inNormal; 
    fragTexCoord=inTexCoord;
}