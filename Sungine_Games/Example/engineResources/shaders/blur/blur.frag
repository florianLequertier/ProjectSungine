
#version 410 core

in block
{
    vec2 Texcoord;
} In;

out vec4 FragColor;
 
uniform sampler2D Texture;
uniform int PassId;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main(void)
{
    vec2 texelSize = vec2(1.0) / textureSize(Texture, 0);
    vec3 result = texture(Texture, In.Texcoord).rgb * weight[0];

    if(PassId == 0)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(Texture, In.Texcoord + vec2(texelSize.x*i, 0)).rgb * weight[i];
            result += texture(Texture, In.Texcoord - vec2(texelSize.x*i, 0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(Texture, In.Texcoord + vec2(0, texelSize.y*i)).rgb * weight[i];
            result += texture(Texture, In.Texcoord - vec2(0, texelSize.y*i)).rgb * weight[i];
        }
    }

    FragColor = vec4(result, 1.0);
}

