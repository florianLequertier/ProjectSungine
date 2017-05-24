
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
//uniform vec3 CameraPosition;
 
 //shadow : 
 uniform sampler2D Shadow;
 uniform mat4 ViewToLightScreen;
 uniform float ShadowFactor;


//lights struct : 

struct SpotLight{
        vec3 direction;
        vec3 position;
        float angle;
        vec3 color;
        float intensity;
};

//light uniforms : 
uniform SpotLight spotLight;

uniform vec2 Resize;


vec3 computeSpotLight(SpotLight light, vec3 p, vec3 n,  vec3 diffuse, vec3 specular, float specularPower)
{
    vec3 l = normalize( light.position - p);
    float ndotl = clamp(dot(n,l), 0.0, 1.0);
    vec3 v = normalize(-p);
    vec3 h = normalize(l+v);
    float ndoth = clamp(dot(n,h),0.0,1.0);
    float d = length(light.position - p);

    float spotFactor = dot( -l, normalize(light.direction));

	diffuse /= 3.1415f;

    specular *= pow(ndoth, specularPower);
    specular /= ( (specularPower + 8.0) / (8.0*3.1415) );

    float intensity = light.intensity / (d*d) ;
    //if(spotFactor > light.angle * 0.5f)
    //    return vec3(0,0,0);

    float teta = dot(-l, normalize(light.direction));
    float phi = cos(light.angle);
    float omega = cos(light.angle*1.3f);

    float falloff = pow( ( teta - phi ) / ( phi - omega ) , 4.f );

	intensity *= clamp(falloff, 0, 1);	

    return intensity * light.color * 3.1415 * (diffuse + specular) * (ndotl) ; 
}


float computeShadow(vec3 p)
{
	//shadow
	float shadowBias = 0.001f;
        vec4 wlP = ViewToLightScreen * vec4(p.xyz, 1.0);
	vec3 lP = vec3(wlP/wlP.w) * 0.5 + 0.5;
	//float lDepth = texture(Shadow, lP.xy).r; //textureProj(Shadow, vec4(lP.xy, lP.z - shadowBias, 1.0), 0.0);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(Shadow, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(Shadow, lP.xy + vec2(x, y)*texelSize).r;
			shadow += (pcfDepth + shadowBias > lP.z) ? 0.0 : 1.0;        
		}    
	}
	shadow /= 9.0;

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
    vec3 color = computeSpotLight(spotLight, p, n, diffuse, specular, specularPower * 100);
    color += ambient;
    color *= (1.0 - (computeShadow(p)*ShadowFactor + ssaoOcclusion));

    Color = vec4(color, ceil(1.0 - depth)); // Transparency at infinity

    float brightness = dot(Color.rgb, vec3(0.2126, 0.7152, 0.0722));
    HighValues = (brightness < 1.0) ? vec4(0.0, 0.0, 0.0, 0.0) : Color;

}
