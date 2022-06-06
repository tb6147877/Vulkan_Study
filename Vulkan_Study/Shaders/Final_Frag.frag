#version 450

layout (binding = 5) uniform sampler2D outputTex;
layout (binding = 6) uniform sampler2D textures[9];

layout (location = 0) in vec2 inUV;
layout (location=1) in flat int instanceID;
layout(location = 0) out vec4 outColor;

void main(){
    //outColor=texture(outputTex,inUV);
    if(inUV.x>=0.5){
        outColor=texture(textures[instanceID],inUV);
    }else{
        outColor=texture(textures[6],inUV);
    }
    
}

