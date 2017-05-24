#version 410 core


precision highp float;
precision highp int;

uniform vec3 Translation;
uniform vec2 Scale;
uniform vec3 CameraRight;
uniform vec3 CameraUp;

uniform mat4 MVP;
//uniform mat4 NormalMatrix;
//uniform vec2 TextureRepetition;

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;
layout(location = 3) in vec3 Tangent;


out block
{
	vec2 TexCoord; 
	vec3 Position;
	vec3 Normal;
} Out;

void main()
{	
	vec3 pos = Position;
	vec3 posWorldSpace = Translation + CameraRight * Position.x * Scale.x + CameraUp * Position.z * Scale.y;

	Out.TexCoord = TexCoord;// * TextureRepetition;
	Out.Position = pos;
	Out.Normal =  Normal;//normalize( vec3(NormalMatrix * vec4(Normal, 0.0)) );

	gl_Position = MVP * vec4(posWorldSpace, 1.0);
}
