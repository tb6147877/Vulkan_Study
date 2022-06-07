#version 450

const vec2 RESOLUTION=vec2(1920,1080);
const int MAX_MARCHING_STEPS=255;
const float MIN_DIST=0.0;
const float MAX_DIST=100.0;
const float EPSILON=0.0001;

//SDF for a cube centered at the origin with width=height=length=2.0
float cubeSDF(vec3 samplePoint){
    vec3 d=abs(samplePoint)-vec3(1.0,1.0,1.0);
    float insideDistance=min(max(d.x,max(d.y,d.z)),0.0);
    float outsideDistance=length(max(d,0.0));
    return insideDistance+outsideDistance;
}


//SDF for a sphere centered at the origin with radius 1.0
float sphereSDF(vec3 samplePoint){
    return length(samplePoint)-1.0;
}

float sceneSDF(vec3 samplePoint){
    return cubeSDF(samplePoint);
    //return sphereSDF(samplePoint);
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

//计算表面上一点P的法线
vec3 estimateNormal(vec3 p){
    return normalize(vec3(
        sceneSDF(vec3(p.x+EPSILON,p.y,p.z))-sceneSDF(vec3(p.x-EPSILON,p.y,p.z)),
        sceneSDF(vec3(p.x,p.y+EPSILON,p.z))-sceneSDF(vec3(p.x,p.y-EPSILON,p.z)),
        sceneSDF(vec3(p.x,p.y,p.z+EPSILON))-sceneSDF(vec3(p.x,p.y,p.z-EPSILON))
    ));
}


/**
 * Lighting contribution of a single point light source via Phong illumination.
 * 
 * The vec3 returned is the RGB color of the light's contribution.
 *
 * k_a: Ambient color
 * k_d: Diffuse color
 * k_s: Specular color
 * alpha: Shininess coefficient
 * p: position of point being lit
 * eye: the position of the camera
 * lightPos: the position of the light
 * lightIntensity: color/intensity of the light
 *
 * See https://en.wikipedia.org/wiki/Phong_reflection_model#Description
 */
vec3 phongContribForLight(vec3 k_d,vec3 k_s,float alpha,vec3 p,vec3 eye,vec3 lightPos,
                            vec3 lightIntensity)
{
    vec3 N=estimateNormal(p);
    vec3 L=normalize(lightPos-p);
    vec3 V=normalize(eye-p);
    vec3 R=normalize(reflect(-L,N));

    float dotLN=dot(L,N);
    float dotRV=dot(R,V);

    if(dotLN<0.0){
        //light not visible from this point to the surface
        return vec3(0.0,0.0,0.0);
    }

    if(dotRV<0.0){
        //apply only diffuse component, since light reflection in opposite direction as viewer
        return lightIntensity*(k_d*dotLN);
    }

    return lightIntensity*(k_d*dotLN+k_s*pow(dotRV,alpha));
}


vec3 phongIllumination(vec3 k_a,vec3 k_d,vec3 k_s,float alpha,vec3 p,vec3 eye){
    const vec3 ambientLight=0.5*vec3(1.0,1.0,1.0);
    vec3 color=ambientLight*k_a;

    vec3 light1Pos=vec3(4.0,2.0,4.0);
    vec3 light1Intensity=vec3(0.4,0.4,0.4);
    color+=phongContribForLight(k_d,k_s,alpha,p,eye,light1Pos,light1Intensity);

    vec3 light2Pos=vec3(2.0,2.0,2.0);
    vec3 light2Intensity=vec3(0.4,0.4,0.4);
    color+=phongContribForLight(k_d,k_s,alpha,p,eye,light2Pos,light2Intensity);

    return color;
}

//return a transform matrix that will transform a view ray from view space to world space,
mat4 viewMatrix(vec3 eye,vec3 target,vec3 up){
    vec3 f=normalize(target-eye);
    vec3 s=normalize(cross(f,up));
    vec3 u=cross(s,f);
    return mat4(
        vec4(s,0.0),
        vec4(u,0.0),
        vec4(-f,0.0),
        vec4(0.0,0.0,0.0,1.0)
    );
}



layout (location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;
void main(){
    
    vec3 viewDir=rayDirection(45.0,RESOLUTION,gl_FragCoord.xy);
    vec3 eye=vec3(8.0,5.0,7.0);
    mat4 viewToWorld=viewMatrix(eye,vec3(0.0,0.0,0.0),vec3(0.0,-1.0,0.0));//the positive direction of y axis of vulkan and opengl in opposite direction 
    vec3 worldDir=(viewToWorld*vec4(viewDir,0.0)).xyz;
    float dist=shortestDistanceToSurface(eye,worldDir,MIN_DIST,MAX_DIST);
    if(dist>MAX_DIST-EPSILON){
        outColor=vec4(0.0,0.0,0.0,0.0);
        return;
    }

    vec3 p=eye+dist*worldDir;
    vec3 k_a=vec3(0.2,0.2,0.2);
    vec3 k_d=vec3(0.7,0.2,0.2);
    vec3 k_s=vec3(1.0,1.0,1.0);
    float shininess=10.0;
    vec3 color=phongIllumination(k_a,k_d,k_s,shininess,p,eye);

    outColor=vec4(color,1.0);
}


