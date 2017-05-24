//lights struct :
struct sPointLight{
        vec3 position;
        vec3 color;
        float intensity;
};

struct sSpotLight{
        vec3 direction;
        vec3 position;
        float angle;
        vec3 color;
        float intensity;
};

struct sDirectionalLight{
        vec3 direction;
        vec3 color;
        float intensity;
};

const int MaxPointLightCount = 10;
const int MaxSpotLightCount = 10;
const int MaxDirectionalLightCount = 5;

//light datas :
uniform mat4 ScreenToView;
uniform sPointLight PointLight[MaxPointLightCount];
uniform sSpotLight SpotLight[MaxSpotLightCount];
uniform sDirectionalLight DirectionalLight[MaxDirectionalLightCount];

uniform int PointLightCount;
uniform int SpotLightCount;
uniform int DirectionalLightCount;

 //material datas :
 uniform sampler2D Diffuse;
 uniform sampler2D Specular;
 uniform sampler2D Bump;
 uniform sampler2D Emissive;
 uniform float SpecularPower;
 uniform float EmissiveMultiplier;
 uniform vec3 EmissiveColor;
 uniform vec4 TintColor;


vec3 computePointLight(sPointLight light, vec3 p, vec3 n,  vec3 diffuse, vec3 specular, float specularPower)
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

///////////////////////////////////////////////////////////////

vec3 computeSpotLight(sSpotLight light, vec3 p, vec3 n,  vec3 diffuse, vec3 specular, float specularPower)
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

///////////////////////////////////////////////////////////////

vec3 computeDirectionalLight(sDirectionalLight light, vec3 p, vec3 n,  vec3 diffuse, vec3 specular, float specularPower)
{
        vec3 l = normalize(-light.direction);
        float ndotl = clamp(dot(n,l), 0.0, 1.0);
        vec3 v = normalize(-p); // Because we are in eye space, otherwise : cameraPosition - p.
        vec3 h = normalize(l+v);
        float ndoth = clamp(dot(n,h),0.0,1.0);

        diffuse /= 3.1415f;

        specular *= pow(ndoth, specularPower);
        specular /= ( (specularPower + 8.0) / (8.0*3.1415) );

        float intensity = light.intensity;

        return intensity * light.color * 3.1415 * (diffuse + specular) * (ndotl) ;
}

///////////////////////////////////////////////////////////////

void main(void)
{
    float depth = gl_FragDepth;
    vec3 paramDiffuse = vec3(1,1,1);
    vec3 paramEmissive = vec3(0,0,0);
    vec3 paramNormals = vec3(0,0,1);
    float paramSpecular = 0.5;
    float paramSpecularPower = 0.5;
    vec2 projectedCoord = vec2(0,0);
    getProjectedCoord(projectedCoord);

    computeShaderParameters(paramDiffuse, paramNormals, paramSpecular, paramSpecularPower, paramEmissive, projectedCoord);

    paramNormals = normalize(paramNormals * 2.0 - 1.0);
    paramNormals = normalize(In.TBN * paramNormals);

    // Convert texture coordinates into screen space coordinates
    vec2 xy = In.TexCoord * 2.0 -1.0;
    // Convert depth to -1,1 range and multiply the point by ScreenToWorld matrix
    vec4 wP = vec4(xy, depth * 2.0 -1.0, 1.0) * ScreenToView;
    // Divide by w
    vec3 p = vec3(wP.xyz / wP.w);

    vec3 ambient = 0.2 * paramDiffuse.rgb;
    vec4 color = vec4(ambient, paramDiffuse.a);

    int pointLightCount = min(MaxPointLightCount, PointLightCount);
    for(int i = 0; i < pointLightCount; ++i)
    {
        color.rgb += computePointLight( PointLight[i], p, paramNormals, diffuse.rgb, paramSpecular, paramSpecularPower * 100 ) / pointLightCount;
    }

    int spotLightCount = min(MaxSpotLightCount, SpotLightCount);
    for(int i = 0; i < spotLightCount ; ++i)
    {
        color.rgb += computeSpotLight( SpotLight[i], p, paramNormals, paramDiffuse.rgb, paramSpecular, paramSpecularPower * 100 ) / spotLightCount;
    }

    int directionalLightCount = min(MaxDirectionalLightCount, DirectionalLightCount);
    for(int i = 0; i < directionalLightCount; ++i)
    {
        color.rgb += computeDirectionalLight( DirectionalLight[i], p, paramNormals, paramDiffuse.rgb, paramSpecular, paramSpecularPower * 100 ) / directionalLightCount;
    }

    Color = vec4(color.rgb, color.a * ceil(1.0 - depth)); // Transparency at infinity

    float brightness = dot(Color.rgb, vec3(0.2126, 0.7152, 0.0722));
    HighValues = (brightness < 1.0) ? vec4(0.0, 0.0, 0.0, 1.0) : Color;
}
