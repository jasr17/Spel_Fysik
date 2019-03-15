// Konstanter
#define SHADOW_EPSILON 0.00001
#define SMAP_WIDTH  1080 * 0.9
#define SMAP_HEIGHT 1920 * 0.9

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

float checkShadowMap(float4 pos, int shadowMapIndex)
{
	// Manually devides by w
	pos.xyz /= pos.w;
	float depth = pos.z;
	// sets coordinates from a [-1, 1] range to [0, 1]. Flips y since directx 0 starts on top
	float2 uvCoord = float2(0.5f * pos.x + 0.5f, -0.5f * pos.y + 0.5f);

	float2 dx = float2(1.0f / SMAP_WIDTH, 1.0f / SMAP_HEIGHT); // size of one texture sample

	// Compares (depth - bias) with the shadowmap
	// 
	float s0 = ((pos.z - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord).r);
	float s1 = ((pos.z - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, 0.0f)).r);
	float s2 = ((pos.z - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(0.0f, dx.y)).r);
	float s3 = ((pos.z - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, dx.y)).r);

	// copy pasta från lektion slide
	//float s0 = (shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord).r + SHADOW_EPSILON < depth) ? 0.0f : 1.0f;
	//float s1 = (shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, 0.0f)).r + SHADOW_EPSILON < depth) ? 0.0f : 1.0f;
	//float s2 = (shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(0.0f, dx.y)).r + SHADOW_EPSILON < depth) ? 0.0f : 1.0f;
	//float s3 = (shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, dx.y)).r + SHADOW_EPSILON < depth) ? 0.0f : 1.0f;

	
	// Transform uv coords to texel space
	float2 texelPos = uvCoord *float2(SMAP_WIDTH, SMAP_HEIGHT);
	//texelPos.x *= SMAP_WIDTH;
	//texelPos.y *= SMAP_HEIGHT;
	float2 lerps = frac(texelPos);
	//return lerps.y;
	float shadowCoeff = lerp( lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);
	
	//shadowCoeff = (s0 + s1 + s2 + s3) / 4;

	return s0;
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
		return checkShadowMap(mul(position, lights[i].viewPerspectiveMatrix), i);
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

				
				finalColor += (lightIntensity * (albeno.xyz * lightColor * diffuseStrength + albeno.xyz * specularStrength * specular.rgb) / pow(distToLight, 0));// *shadowCoeff;
            }
        }
    }

	float4 pos = mul(position, lights[0].viewPerspectiveMatrix);
	pos.xyz /= pos.w;
	float depth = pos.z;


	float2 dx = float2(1.0f / SMAP_WIDTH, 1.0f / SMAP_HEIGHT); // size of one texture sample
	float2 uvCoord = float2(0.5f * pos.x + 0.5f, -0.5f * pos.y + 0.5f);
	float2 texelPos = uvCoord * float2(SMAP_WIDTH, SMAP_HEIGHT);

	float s = shadowMap[0].Sample(AnisoSampler, uvCoord).r;
	return float4(s, s, s, 1);
	// 
	//float s0 = ((pos.z - SHADOW_EPSILON) < shadowMap[0].Sample(AnisoSampler, uvCoord).r);
	//float s1 = ((pos.z - SHADOW_EPSILON) < shadowMap[0].Sample(AnisoSampler, uvCoord + float2(dx.x, 0.0f)).r);
	//float s2 = ((pos.z - SHADOW_EPSILON) < shadowMap[0].Sample(AnisoSampler, uvCoord + float2(0.0f, dx.y)).r);
	//float s3 = ((pos.z - SHADOW_EPSILON) < shadowMap[0].Sample(AnisoSampler, uvCoord + float2(dx.x, dx.y)).r);
	//return float4(s0, s1, s2, s3);

	//// Grid for seing shadowmap texels
	if (frac(texelPos.x) < 0.1 || frac(texelPos.y) < 0.1)
		return float4(0, 0.3, 0, 1);

	return clamp(float4(finalColor,1),0,1);
}