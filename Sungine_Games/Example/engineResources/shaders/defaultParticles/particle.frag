#version 410 core

#define POSITIONS 0
#define VELOCITIES 1
#define FORCES 2
#define ELAPSED_TIMES 3
#define LIFE_TIMES 4
#define COLORS 5
#define SIZES 6

uniform sampler2D Texture;

layout(location = 0) out vec4 FragColor;

in block
{
    vec2 TexCoord;
} In;

void main()
{
	//no lighting for the moment 

	FragColor = vec4(1,0,0,1);// texture2D(Texture, In.TexCoord) * In.Color;
}
