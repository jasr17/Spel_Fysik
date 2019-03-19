// Konstanter
#define SHADOW_EPSILON (0.000006)
#define SHADOW_EPSILON2 (0.999997)

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
	float4 smapSize;  // 
};
cbuffer cameraBuffer : register(b1)
{
	float4 camPos;
}

//Texturer/samplers
Texture2D Textures[4]	: register(t0);
Texture2D shadowMap[10] : register(t10);
SamplerState AnisoSampler;

float checkShadowMap(float4 pos, int shadowMapIndex)
{
	// Manually devides by w
	pos.xyz /= pos.w;
	float depth = pos.z;
	// sets coordinates from a [-1, 1] range to [0, 1]. Flips y since directx 0 starts on top
	float2 uvCoord = float2(0.5f * pos.x + 0.5f, -0.5f * pos.y + 0.5f);

	float2 mapSize = float2(smapSize.x, smapSize.y);
	float2 dx = float2(1.0f / smapSize.x, 1.0f / smapSize.y); // size of one texture sample
	
	float2 texelPos = uvCoord * mapSize; 
	float2 lerps = frac(texelPos); // Posision on the texel. Used in linear interpolation to judge weight of contributon
	

	// Aligns uvCoords 
	uvCoord = (texelPos - lerps) / mapSize;

	// Compares (depth - bias) with the shadowmap on 4 neighburing texels
	float s0 = ((pos.z * SHADOW_EPSILON2) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord).r);
	float s1 = ((pos.z * SHADOW_EPSILON2) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, 0.0f)).r);
	float s2 = ((pos.z * SHADOW_EPSILON2) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(0.0f, dx.y)).r);
	float s3 = ((pos.z * SHADOW_EPSILON2) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, dx.y)).r);
	
	// Interpolates the result of the sampling
	float shadowCoeff = lerp( lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);
	
	// Very blocky version
	//shadowCoeff = (s0 + s1 + s2 + s3) / 4;
	
	return shadowCoeff;
}

struct PixelShaderInput {
	float4 Pos	    : SV_Position;
	float2 uv		: TEXCOORDS;
};

//G-bufferns pixelshader
float4 PS_main(in PixelShaderInput input) : SV_TARGET
{
	float4 normal	= normalize(Textures[0].Sample(AnisoSampler, input.uv));
	float4 albeno	= Textures[1].Sample(AnisoSampler, input.uv);
	float4 position = Textures[2].Sample(AnisoSampler, input.uv);
    float4 specular = Textures[3].Sample(AnisoSampler, input.uv);

	float3 ambient = float3(0.2, 0.2, 0.2);
    float3 finalColor = albeno.xyz * ambient;
	for (int i = 0; i < lightCount.x; i++)
    {
		float shadowCoeff = checkShadowMap(mul(position, lights[i].viewPerspectiveMatrix), i);
        if (shadowCoeff > 0)
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
								
				finalColor += (lightIntensity * (albeno.xyz * lightColor * diffuseStrength + albeno.xyz * specularStrength * specular.rgb) / pow(distToLight, 0)) * shadowCoeff;
            }
        }
    }
	return clamp(float4(finalColor,1),0,1);
}