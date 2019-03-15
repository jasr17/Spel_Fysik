// Konstanter
struct ShaderLight
{
	float4 position;
	float4 color; //.a is intensity
	float4x4 viewPerspectiveMatrix;
};
cbuffer lightBuffer : register(b0)
{
	float4 lightCount;
	ShaderLight lights[10];
};
cbuffer cameraBuffer : register(b1)
{
	float4 camPos;
}

//Texturer/samplers
Texture2D Textures[4]	: register(t0);
Texture2D shadowMap[10] : register(t10);
SamplerState AnisoSampler;

//SSAO
//-----------------------------------------------------//
float randomSize;
float sampleRad;
float intensity;
float scale;
float bias;
//Texture2D NoiceMap	: register();

float DoAmbientOcclusion(in float2 texCoord, in float2 uv, in float3 p, in float3 normal)
{
	float3 diff = Textures[2].AnisoSampler();
}

//----------------------------------------------------//

bool checkShadowMap(float4 pos, int shadowMapIndex)
{
	// Manually devides by w
	pos.xyz /= pos.w;
	// sets coordinates from a [-1, 1] range to [0, 1]. Flips y since directx 0 starts on top
	float2 uvCoord = float2(0.5f*pos.x + 0.5f, -0.5f*pos.y + 0.5f);

	// Compares (depth - bias) with the shadowmap
	return (pos.z - 0.00001 < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord).r);
}

struct PixelShaderInput {
	float4 Pos	    : SV_Position;
	float2 uv		: TEXCOORDS;
};

//G-bufferns pixelshader
float4 PS_main(in PixelShaderInput input) : SV_TARGET
{
	float4 normal	= normalize(Textures[0].Sample(AnisoSampler, input.uv));
	float4 color	= Textures[1].Sample(AnisoSampler, input.uv);
	float4 position = Textures[2].Sample(AnisoSampler, input.uv);
    float4 specular = Textures[3].Sample(AnisoSampler, input.uv);

	float3 ambient = float3(0.2, 0.2, 0.2);
    float3 finalColor = color.xyz * ambient;
	for (int i = 0; i < lightCount.x; i++)
    {
        if (checkShadowMap(mul(position, lights[i].viewPerspectiveMatrix), i))
        {

            float3 lightPos = lights[i].position.xyz;
            float3 lightColor = lights[i].color.rgb;
            float lightIntensity = lights[i].color.a;

            float3 toLight = normalize(lightPos - position.xyz);
            float dotNormaltoLight = dot(normal.xyz, toLight); //dot(normal, toLight) if less than one then the triangle is facing the other way, ignore
            if (dotNormaltoLight > 0)
            {
			    //diffuse
                float distToLight = length(lightPos - position.xyz);
                float diffuseStrength = dotNormaltoLight;
			   	//specular
                float3 toCam = normalize(camPos.xyz - position.xyz);
                float3 reflekt = normalize(2 * dotNormaltoLight * normal.xyz - toLight);
                float specularStrength = pow(max(dot(reflekt, toCam), 0), specular.w);


                finalColor += lightIntensity * (color.xyz * lightColor * diffuseStrength + color.xyz * specularStrength * specular.rgb) / pow(distToLight, 0);
            }
        }
    }
	return clamp(float4(finalColor,1),0,1);
}