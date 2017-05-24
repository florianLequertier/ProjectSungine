#version 410 core

#define POSITION	0
#define NORMAL		1
#define TEXCOORD	2
#define TANGENT		3
#define FRAG_COLOR	0

precision highp int;

// inputs : 
in block
{
        vec2 TexCoord;
        vec3 Position;
        vec4 ClipSpaceCoord;
        mat3 TBN;
} In;

// Outputs : 

// Write in GL_COLOR_ATTACHMENT0
layout(location = 0 ) out vec4 outColor;
// Write in GL_COLOR_ATTACHMENT1
layout(location = 1) out vec4 outNormal;
// Write in GL_COLOR_ATTACHMENT2
layout(location = 2) out vec4 outHighValues;
// Write in GL_COLOR_ATTACHMENT3
layout(location = 3) out vec4 outPositions;

// Uniforms : 
uniform sampler2D Diffuse;
uniform sampler2D Specular;
uniform sampler2D Bump;
uniform sampler2D Emissive;
uniform float SpecularPower;
uniform float EmissiveMultiplier;
uniform vec3 EmissiveColor;
uniform vec3 TintColor;

void main()
{

        outColor = vec4( texture(Diffuse, In.TexCoord).rgb * TintColor.rgb, texture(Specular, In.TexCoord).r );
        outHighValues = vec4(texture(Emissive, In.TexCoord).rgb * EmissiveColor.rgb * vec3(EmissiveMultiplier), 1.0);

	vec3 bumpNormal = texture(Bump, In.TexCoord).rgb;
	bumpNormal = normalize(bumpNormal * 2.0 - 1.0);
	bumpNormal = normalize(In.TBN * bumpNormal);
        outNormal = vec4( bumpNormal*0.5+0.5, SpecularPower/100.0 );

        outPositions = vec4(In.Position, 1.0);
}
