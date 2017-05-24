#version 410 core

#define POSITION 0

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

// Outputs :
out block
{
    vec2 Texcoord;
    flat int FlareIdx;
    vec3 FlareColor;
    float FlareIntensity;
} Out;

in geomDatas
{
    vec4 LightPosClipSpace;
    vec3 FlareColor;
    float FlareIntensity;
} In[];

void makeFlare(vec2 screenPosition, float halfWidth, float halfHeight)
{    
    vec2 offset = vec2(-halfWidth, -halfHeight);
    gl_Position = vec4(screenPosition + offset, 0, 1);
    Out.Texcoord = vec2(0.0, 0.0);
    EmitVertex();

    offset = vec2(-halfWidth, halfHeight);
    gl_Position = vec4(screenPosition + offset, 0, 1);
    Out.Texcoord = vec2(0.0, 1.0);
    EmitVertex();

    offset = vec2(halfWidth, -halfHeight);
    gl_Position = vec4(screenPosition + offset, 0, 1);
    Out.Texcoord = vec2(1.0, 0.0);
    EmitVertex();

    offset = vec2(halfWidth, halfHeight);
    gl_Position = vec4(screenPosition + offset, 0, 1);
    Out.Texcoord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}

uniform sampler2D Depth;
uniform float TrailLength;
uniform float TrailCollapsePos;
uniform float TrailSize;
uniform vec2 MainFlareSize;
uniform float FlareOffsetMultiplier;

//uniform mat4 VP;

const int trailCount = 6;

void main()
{
    Out.FlareColor = In[0].FlareColor;
    Out.FlareIntensity = In[0].FlareIntensity;

   vec4 lightPosClipSpace = In[0].LightPosClipSpace;// * gl_in[0].gl_Position;
   float lightDepth = (lightPosClipSpace.z / lightPosClipSpace.w) * 0.5 + 0.5;
   vec2 lightPosScreenSpace = lightPosClipSpace.xy / lightPosClipSpace.w;
   vec2 lightPosTextureSpace = lightPosScreenSpace * 0.5 + 0.5;
   vec2 texSize = textureSize(Depth, 0);
   float yRatio = texSize.x / texSize.y;

   if(abs(lightPosScreenSpace.x) > 1.0 || abs(lightPosScreenSpace.y) > 1.0 || texture(Depth, lightPosTextureSpace).r < lightDepth)
       return;

   float modifiedTrailCollapsePos = TrailCollapsePos + (length(lightPosScreenSpace) * FlareOffsetMultiplier);
   vec2 trailDir = normalize(-lightPosScreenSpace);
   vec2 tailCollapsePoint = (lightPosScreenSpace + trailDir * modifiedTrailCollapsePos);
   float sizeCoeff = TrailSize / (modifiedTrailCollapsePos == 0.0 ? 0.001 : modifiedTrailCollapsePos);
   float distToCenter = length(lightPosScreenSpace);
   vec2 uStep = (trailDir * TrailLength) / trailCount;

   vec2 uTot = vec2(0, 0);
   vec2 flarePos = lightPosScreenSpace + uTot;
   float distToTCpoint = distance(flarePos, tailCollapsePoint);
   float flareHalfSize = distToTCpoint * sizeCoeff;

   //First flare
   Out.FlareIdx = 0;
   makeFlare(flarePos, MainFlareSize.x, MainFlareSize.y*yRatio);

   //Flares forming the trail
   for(int i = 0; i < trailCount; i++)
   {
       Out.FlareIdx = i+1;
       uTot += uStep;
       flarePos = lightPosScreenSpace + uTot;
       distToTCpoint = distance(flarePos, tailCollapsePoint);
       flareHalfSize = distToTCpoint * sizeCoeff;

       flarePos = lightPosScreenSpace + uTot * distToCenter;

       makeFlare(flarePos, flareHalfSize, flareHalfSize*yRatio);
   }
}
