#version 410 core

// Outputs : 
layout(location = 0) out vec4 outColor;

uniform sampler2D ReflexionTexture;

in vec2 NormalizedPosition2D;

void main(void)
{
    //vec2 texCoord = gl_FragCoord.xy / textureSize(ReflexionTexture, 0);
    vec3 color = texture(ReflexionTexture, NormalizedPosition2D).rgb;
    outColor = vec4(color, 1.0);
}
