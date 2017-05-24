#version 410 core

#define FRAG_COLOR	0

precision highp int;

// inputs : 

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 HightValuesColor;

in block
{
	vec3 TexCoord;
} In;

// Uniforms : 

uniform samplerCube Diffuse;


void main()
{
    FragColor = vec4(texture(Diffuse, In.TexCoord).rgb, 1);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    HightValuesColor = (brightness < 1.0) ? vec4(0.0, 0.0, 0.0, 0.0) : FragColor;
}
