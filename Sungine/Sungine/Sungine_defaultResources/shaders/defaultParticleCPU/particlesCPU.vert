#version 410 core

precision highp float;
precision highp int;

#define POSITIONS 0
#define NORMALS 1
#define UVS 2
#define TRANSLATIONS 3
#define COLORS 4
#define SIZES 5


layout(location = POSITIONS) in vec3 Position;
layout(location = NORMALS) in vec3 Normal;
layout(location = UVS) in vec2 TexCoord;
layout(location = TRANSLATIONS) in vec3 Translation;
layout(location = COLORS) in vec4 Color;
layout(location = SIZES) in vec2 Size;

uniform vec3 CameraRight;
uniform vec3 CameraUp;
uniform mat4 VP;

out block
{
	vec2 TexCoord; 
	vec3 Position;
	vec3 Normal;
	vec4 Color;
} Out;

void main()
{	
	Out.Color = Color;

	vec3 pos = Position;
	vec3 posWorldSpace = Translation + CameraRight * Position.x * Size.x + CameraUp * Position.z * Size.y;

	Out.TexCoord = TexCoord;// * TextureRepetition;
	Out.Position = pos;
	Out.Normal =  Normal;//normalize( vec3(NormalMatrix * vec4(Normal, 0.0)) );

	gl_Position = VP * vec4(posWorldSpace, 1.0);
}
