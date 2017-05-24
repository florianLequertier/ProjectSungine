#version 410 core

// Inputs : 
in block
{
    vec2 Texcoord;
    flat int FlareIdx;
    vec3 FlareColor;
    float FlareIntensity;
} In; 

// Outputs : 
layout(location = 0) out vec4  outColor;

//Uniforms : 
uniform sampler2D MainFlare;
uniform sampler2D TrailFlare;
uniform float Intensity;
uniform float MinIntensity;

void main(void)
{
    vec3 color = In.FlareIdx == 0 ? texture(MainFlare, In.Texcoord).rgb : texture(TrailFlare, In.Texcoord).rgb; // Texture color
    color *= In.FlareColor.rgb; // Tint color
    color *= clamp((In.FlareIntensity * Intensity + MinIntensity), 0.0, 1.0); // Intensity
    outColor = vec4(color, 1.0);
}
