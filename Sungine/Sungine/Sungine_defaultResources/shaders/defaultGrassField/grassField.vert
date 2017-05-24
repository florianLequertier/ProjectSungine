#version 410 core

#define POSITION	0
#define NORMAL		1
#define TEXCOORD	2
#define TRANSLATION	3
#define ANIM_POS	4

precision highp float;
precision highp int;

layout(location = POSITION) in vec3 Position;
layout(location = NORMAL) in vec3 Normal;
layout(location = TEXCOORD) in vec2 TexCoord;
layout(location = TRANSLATION) in vec3 Translation;
layout(location = ANIM_POS) in vec3 animPos;

uniform float Time;
uniform mat4 VP;


out block
{
	vec2 TexCoord; 
	vec3 Position;
	vec3 Normal;
} Out;

void main()
{	
	vec3 pos = Position;

	if(pos.y > 0.1f) //simple test to get up vertices only
	{
		pos += animPos;
	}

	Out.TexCoord = TexCoord;
	Out.Position = pos;
	Out.Normal =  Normal; //normalize( vec3(NormalMatrix * vec4(Normal, 0)) );

	gl_Position = VP * vec4(pos + Translation,1);
}
