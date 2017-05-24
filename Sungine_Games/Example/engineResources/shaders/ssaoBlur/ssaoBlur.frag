#version 410 core

// Inputs : 
in block
{
    vec2 Texcoord;
} In; 

// Outputs : 
layout(location = 0) out float outColor;

//Uniforms : 
uniform sampler2D Texture;

void main(void)
{
    vec2 texelSize = 1.0 / textureSize(Texture, 0);
    float result = 0.0;

    for(int j = -2; j < 2; j++)
    {
        for(int i = -2; i < 2; i++)
        {
            vec2 offset = vec2(float(i),float(j)) * texelSize;
            result += texture(Texture, In.Texcoord + offset).r;
        }
    }
    result /= 16.0;

    outColor = result;
}
