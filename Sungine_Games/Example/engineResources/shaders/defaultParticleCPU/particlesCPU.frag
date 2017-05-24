#version 410 core

#define POSITIONS 0
#define NORMALS 1
#define UVS 2
#define TRANSLATIONS 3
#define COLORS 4
#define SIZES 5

uniform sampler2D Texture;

layout(location = 0) out vec4 FragColor;

in block
{
        vec2 TexCoord;
        vec3 Position;
        vec3 Normal;
		vec4 Color;
} In;

void main()
{
	//no lighting for the moment 

	FragColor = texture2D(Texture, In.TexCoord) * In.Color;
	if(FragColor.a == 0)
		discard;
}
