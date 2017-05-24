#version 410 core

precision highp float;
precision highp int;

// Inputs : 
in block
{
    vec2 Texcoord;
	vec2 RepeatedTexcoord;
} In; 


// Write in GL_COLOR_ATTACHMENT0
layout(location = 0 ) out vec4 outDiffuse;
// Write in GL_COLOR_ATTACHMENT1
layout(location = 1) out vec4 outBump;
// Write in GL_COLOR_ATTACHMENT2
layout(location = 2) out vec4 outSpecular;

//Uniforms : 
uniform sampler2D FilterTexture;
uniform sampler2D Bump;
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform vec2 FilterValues;

void main(void)
{

    float filterColor = texture(FilterTexture, In.Texcoord).r;
	if(filterColor >= (FilterValues.x) && filterColor < (FilterValues.y))
	{
		outDiffuse = vec4(texture(Diffuse, In.RepeatedTexcoord).rgb, 1.0);
		outBump = vec4(texture(Bump, In.RepeatedTexcoord).rgb, 1.0);
		outSpecular = vec4(texture(Specular, In.RepeatedTexcoord).rgb, 1.0);
	}
	else
	{
		discard;
	}
}