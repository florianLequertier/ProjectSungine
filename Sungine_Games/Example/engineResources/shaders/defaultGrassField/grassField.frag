#version 410 core

#define FRAG_COLOR	0

precision highp int;

//layout(location = FRAG_COLOR) out vec4 FragColor;

// Write in GL_COLOR_ATTACHMENT0
layout(location = 0 ) out vec4 outColor;
// Write in GL_COLOR_ATTACHMENT1
layout(location = 1) out vec4 outNormal;

uniform sampler2D Texture;

in block
{
	vec2 TexCoord;
    vec3 Position;
	vec3 Normal;
} In;


//void main()
//{
//	vec4 tmpColor = texture2D(Texture, In.TexCoord);
//	if(tmpColor.a <= 0.001)
//		discard;
//	FragColor = tmpColor;
//}


void main()
{
	vec4 tmpColor = texture(Texture, In.TexCoord);
	if(tmpColor.a <= 0.1)
	{
		discard;
	}
	else
	{
		outColor = vec4( tmpColor.rgb, 0.5 ); //specular color at 0.5
		outNormal = vec4( In.Normal*0.5+0.5, 0.5 ); //specular power at 0.5
	}
}
