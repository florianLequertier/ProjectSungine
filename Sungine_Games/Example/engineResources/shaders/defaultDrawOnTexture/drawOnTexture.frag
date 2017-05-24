#version 410 core

precision highp float;
precision highp int;

// Inputs : 
in block
{
    vec2 Texcoord;
	vec2 Position;
} In; 

// Outputs : 
layout(location = 0) out vec4 outColor;

//Uniforms : 
uniform sampler2D Texture;
uniform vec4 DrawColor;
uniform vec2 DrawPosition;
uniform float DrawRadius;

void main(void)
{
	vec4 tmpColor = vec4(0,0,0,0);
    vec4 texColor = texture2D(Texture, In.Texcoord);
	float d = length(In.Position - DrawPosition);

	if(d < 0.9*DrawRadius)
		tmpColor = DrawColor;
	else if(d > 0.9*DrawRadius && d < DrawRadius)
		tmpColor = DrawColor * ( 1 - (d - 0.9*DrawRadius)/(0.1*DrawRadius) ) + texColor * (d - 0.9*DrawRadius)/(0.1*DrawRadius); // linear attenuation from 0.9*drawRadius to drawRadius
	else
		tmpColor = texColor;

    outColor = tmpColor;
}