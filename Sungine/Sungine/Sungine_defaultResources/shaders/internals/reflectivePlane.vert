#define POSITION	 0
#define NORMAL		 1
#define TEXCOORD	 2
#define TANGENT		 3
#define BONE_IDS	 4
#define BONE_WEIGHTS 5

#define FRAG_COLOR	0

const unsigned int MAX_BONE_COUNT = 100;

//uniform mat4 MVP;
//uniform mat4 NormalMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform vec2 TextureRepetition;

layout(location = POSITION) in vec3 Position;
layout(location = NORMAL) in vec3 Normal;
layout(location = TEXCOORD) in vec2 TexCoord;
layout(location = TANGENT) in vec3 Tangent;

out block
{
        vec2 TexCoord;
        vec3 Position;
        vec4 ClipSpaceCoord;
        mat3 TBN;
} Out;

void main()
{
    mat4 normalMatrix = transpose(inverse(ViewMatrix * ModelMatrix));

    //calculate TBN matrix :
    vec3 T = normalize( vec3(normalMatrix * vec4(Tangent, 0.0)) );
    vec3 N = normalize( vec3(normalMatrix * vec4(Normal, 0.0)) );
    vec3 B = -cross(T, N);
    Out.TBN = mat3(B, T, N);

    Out.TexCoord = TexCoord * TextureRepetition;
    Out.Position = (ViewMatrix * vec4(Position, 1)).rgb;

    gl_ClipDistance[0] = -1; //always invisible for clipped reflections

    Out.ClipSpaceCoord = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position,1);
    gl_Position = Out.ClipSpaceCoord;

}
