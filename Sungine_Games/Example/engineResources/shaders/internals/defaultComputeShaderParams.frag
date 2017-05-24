//// Uniforms :
//uniform sampler2D Diffuse;
//uniform sampler2D Specular;
//uniform sampler2D Bump;
//uniform sampler2D Emissive;
//uniform float SpecularPower;
//uniform float EmissiveMultiplier;
//uniform vec3 EmissiveColor;
//uniform vec3 TintColor;

void computeShaderParameters(inout vec3 paramDiffuse, inout vec3 paramNormals, inout float paramSpecular, inout float paramSpecularPower, inout vec3 paramEmissive, in vec2 projectedCoord)
{
//    paramDiffuse = texture(Diffuse, In.TexCoord).rgb * TintColor.rgb;
//    paramSpecular = texture(Specular, In.TexCoord).r;
//    paramEmissive = texture(Emissive, In.TexCoord).rgb * EmissiveColor.rgb * vec3(EmissiveMultiplier);
//    paramNormals = texture(Bump, In.TexCoord).rgb;
//    paramSpecularPower = SpecularPower;
}
