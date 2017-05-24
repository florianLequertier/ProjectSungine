
#version 410 core

in block
{
    vec2 Texcoord;
} In;

layout(location = 0)out vec4 Color;
layout(location = 1)out vec4 HighValues;

// Uniforms : 


uniform sampler2D ColorBuffer;
uniform sampler2D NormalBuffer;
uniform sampler2D DepthBuffer;
uniform sampler2D SSAOTexture;
uniform mat4 ScreenToView;
uniform mat4 ViewToWorld;
//uniform vec3 CameraPosition;
 
 //shadow : 
 uniform samplerCube Shadow;
 uniform float FarPlane;
 uniform float ShadowFactor;

//lights struct : 

struct PointLight{
	vec3 position;
	vec3 color;
	float intensity; 
};

//light uniforms : 
uniform PointLight pointLight;

uniform vec2 Resize;


vec3 computePointLight(PointLight light, vec3 p, vec3 n,  vec3 diffuse, vec3 specular, float specularPower)
{
	vec3 l = normalize(light.position - p);
	float ndotl = clamp(dot(n,l), 0.0, 1.0);
        vec3 v = normalize(-p);
	vec3 h = normalize(l+v);
	float ndoth = clamp(dot(n,h),0.0,1.0);
	float d = length(light.position - p);

	diffuse /= 3.1415f;

        specular *= pow(ndoth, specularPower);
	specular /= ( (specularPower + 8.0) / (8.0*3.1415) );

        //specular = clamp(specular, 0, 100);

	float intensity = light.intensity / (d*d);

        return intensity * light.color * 3.1415 * (diffuse + specular) * (ndotl) ;
}

vec3 kernelSoft[20] = vec3[](

   vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
   vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
   vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
   vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)

);

float computeShadow(vec3 p)
{
    //shadow
    float shadowBias = 0.01;
    //We have to compute the ViewToWorld because we compute the shadowMap in worldSpace.
    vec3 pWorld = vec3(ViewToWorld * vec4(p, 1.0));
    vec3 lightWorld = vec3(ViewToWorld * vec4(pointLight.position, 1.0));
    vec3 lightToFrag = pWorld - lightWorld;

    float clothestDepth = texture(Shadow, lightToFrag).r;
    clothestDepth *= FarPlane;
    float currentDepth = length(lightToFrag);

    float shadow = 0.0; //(clothestDepth + shadowBias > currentDepth) ? 0.0 : 1.0;

    for(int i=0; i<20; i++)
    {
                lightToFrag = normalize((pWorld - lightWorld) + kernelSoft[i]*0.05);
                clothestDepth = texture(Shadow, lightToFrag).r;
                clothestDepth *= FarPlane;

                shadow += (clothestDepth + shadowBias > currentDepth) ? 0.0 : 1.0;
    }
    shadow /= 20.0;

    return shadow;
}

void main(void)
{
    // Read gbuffer values
    vec4 colorBuffer = texture(ColorBuffer, In.Texcoord).rgba;
    vec4 normalBuffer = texture(NormalBuffer, In.Texcoord).rgba;
    float depth = texture(DepthBuffer, In.Texcoord).r;

    // Convert texture coordinates into screen space coordinates
    vec2 xy = (In.Texcoord / Resize) * 2.0 -1.0;
    // Convert depth to -1,1 range and multiply the point by ScreenToWorld matrix
    vec4 wP = vec4(xy, depth * 2.0 -1.0, 1.0) * ScreenToView;
    // Divide by w
    vec3 p = vec3(wP.xyz / wP.w);

    vec3 diffuse = colorBuffer.rgb;
    vec3 specular = colorBuffer.aaa;
    float specularPower = normalBuffer.a;
    vec3 n = normalBuffer.rgb*2.0 -1.0;

    float ssaoOcclusion = texture(SSAOTexture, In.Texcoord).r;

    vec3 ambient = 0.2 * diffuse * (1.0 - ssaoOcclusion);
    vec3 color = computePointLight( pointLight, p, n, diffuse, specular, specularPower * 100 );
    color += ambient;
    color *= (1.0 - (computeShadow(p)*ShadowFactor + ssaoOcclusion));

    Color = vec4(color, ceil(1.0 - depth)); // Transparency at infinity

    float brightness = dot(Color.rgb, vec3(0.2126, 0.7152, 0.0722));
    HighValues = (brightness < 1.0) ? vec4(0.0, 0.0, 0.0, 1.0) : Color;
}
