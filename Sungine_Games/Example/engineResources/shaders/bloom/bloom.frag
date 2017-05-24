
#version 410 core

in block
{
    vec2 Texcoord;
} In;

out vec4 FragColor;
 
uniform sampler2D Texture;
uniform sampler2D TextureBlur;
uniform float Exposure;
uniform float Gamma;

void main(void)
{

    vec3 hdrColor = texture(Texture, In.Texcoord).rgb;
    vec3 bloomColor = texture(TextureBlur, In.Texcoord).rgb;
    hdrColor += bloomColor;
    // Tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * Exposure);
    // Gamma correction
    result = pow(result, vec3(1.0/ Gamma));

    FragColor = vec4(result, 1.0);

}

