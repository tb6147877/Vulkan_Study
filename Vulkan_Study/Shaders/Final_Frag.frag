#version 450
/*
layout (binding = 5) uniform sampler2D outputTex;

layout (location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main(){
    outColor=texture(outputTex,inUV);
}
*/

const vec2 RESOLUTION=vec2(1920,1080);
const int MAX_MARCHING_STEPS=255;
const float MIN_DIST=0.0;
const float MAX_DIST=100.0;
const float EPSILON=0.0001;

float sphereSDF(vec3 samplePoint){
    return length(samplePoint)-1.0;
}

float sceneSDF(vec3 samplePoint){
    return sphereSDF(samplePoint);
}

float shortestDistanceToSurface(vec3 eye,vec3 marchingDir,float start,float end){
    float depth=start;
    for(int i=0;i<MAX_MARCHING_STEPS;i++){
        float dist=sceneSDF(eye+depth*marchingDir);
        if(dist<EPSILON){return depth;}
        depth+=dist;
        if(depth>=end){return end;}
    }
    return end;
}

vec3 rayDirection(float fov,vec2 size,vec2 fragCoord){
    vec2 xy=fragCoord-size/2.0;
    float z=size.y/tan(radians(fov)/2.0);
    return normalize(vec3(xy,-z));
}



layout (location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
void main(){
    
    vec3 dir=rayDirection(45.0,RESOLUTION,gl_FragCoord.xy);
    vec3 eye=vec3(0.0,0.0,5.0);
    float dist=shortestDistanceToSurface(eye,dir,MIN_DIST,MAX_DIST);
    if(dist>MAX_DIST-EPSILON){
        outColor=vec4(0.0,0.0,0.0,0.0);
        return;
    }

    outColor=vec4(1.0,1.0,0.0,1.0);
}


