// Konstanter
#define SHADOW_EPSILON 0.00001
#define SMAP_WIDTH  (1080 * 0.9)
#define SMAP_HEIGHT (1920 * 0.9)

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
cbuffer kernelBuffer : register(b2) {
	float4 kernels[8];
	float nrOfKernels;
}

//Texturer/samplers
Texture2D Textures[5]	: register(t0);
Texture2D Noise			: register(t5);
Texture2D shadowMap[10] : register(t10);
SamplerState AnisoSampler;

//SSAO
//-----------------------------------------------------//
//Texture2D NoiseMap :register ();
//Först hitta pixelns position i viewspace och dess normal (textures2).

float3x3 SSAO_TBN(in float3 randomVector, in float3 normal) {
	float3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
	float3 bitangent = cross(normal, tangent);
	return float3x3(tangent, bitangent, normal);
}
float occlusion()
{
	float occlusion = 0.0;
	/*
	for(int i = 0; i < kernelSize;i++){

	*/
}
//-------------
//Rotation functions



//----------------------------------------------------//


float checkShadowMap(float4 pos, int shadowMapIndex)
{
	// Manually devides by w
	pos.xyz /= pos.w;
	float depth = pos.z;
	// sets coordinates from a [-1, 1] range to [0, 1]. Flips y since directx 0 starts on top
	float2 uvCoord = float2(0.5f * pos.x + 0.5f, -0.5f * pos.y + 0.5f);

	float2 mapSize = float2(SMAP_WIDTH, SMAP_HEIGHT);
	float2 dx = float2(1.0f / SMAP_WIDTH, 1.0f / SMAP_HEIGHT); // size of one texture sample
	
	float2 texelPos = uvCoord * mapSize; 
	float2 lerps = frac(texelPos); // Posision on the texel. Used in linear interpolation to judge weight of contributon

	// Aligns uvCoords 
	uvCoord = (texelPos - lerps) / mapSize;

	// Compares (depth - bias) with the shadowmap on 4 neighburing texels
	float s0 = ((pos.z - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord).r);
	float s1 = ((pos.z - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, 0.0f)).r);
	float s2 = ((pos.z - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(0.0f, dx.y)).r);
	float s3 = ((pos.z - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, dx.y)).r);

	// Interpolates the result of the sampling
	float shadowCoeff = lerp( lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);
	
	// Very blocky version
	//shadowCoeff = (s0 + s1 + s2 + s3) / 4;

	return shadowCoeff;
}

struct PS_IN {
	float4 Pos	    : SV_Position;
	float2 uv		: TEXCOORDS;
};

//G-bufferns pixelshader
float4 PS_main(in PS_IN input) : SV_TARGET
{
	float4 normal	= normalize(Textures[0].Sample(AnisoSampler, input.uv));
	float4 color	= Textures[1].Sample(AnisoSampler, input.uv);
	float4 position = Textures[2].Sample(AnisoSampler, input.uv);
    float4 specular = Textures[3].Sample(AnisoSampler, input.uv);
	float4 viewPos	= Textures[4].Sample(AnisoSampler, input.uv);
	float4 randomVec = Noise.Sample(AnisoSampler, input.uv);
	
	
	//OM KONSTIG BILD; BYT PLATS PÅ VIEWPOS OCH SSAO_TBN
	float3 TBNVec = mul(viewPos.xyz, SSAO_TBN(normalize(float3(1, 1, 1))/*randomVector*/, normal));



	//-------------------------------------
	//SSAO
	//------------------------------------
	//To calculate the occlusion we need the pixels location in viewSpace
	float3 origin;

	//---------------------------------------


	float3 ambient = float3(0.2, 0.2, 0.2);
    float3 finalColor = color.xyz * ambient;
	for (int i = 0; i < lightCount.x; i++)
    {
		float shadowCoeff = checkShadowMap(mul(position, lights[i].viewPerspectiveMatrix), i);
		//return checkShadowMap(mul(position, lights[i].viewPerspectiveMatrix), i);
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

				
				finalColor += (lightIntensity * (color.xyz * lightColor * diffuseStrength * shadowCoeff + color.xyz * specularStrength * specular.rgb) / pow(distToLight, 0));
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
	//return float4(s, s, s, 1);
	// 
	//float s0 = ((pos.z - SHADOW_EPSILON) < shadowMap[0].Sample(AnisoSampler, uvCoord).r);
	//float s1 = ((pos.z - SHADOW_EPSILON) < shadowMap[0].Sample(AnisoSampler, uvCoord + float2(dx.x, 0.0f)).r);
	//float s2 = ((pos.z - SHADOW_EPSILON) < shadowMap[0].Sample(AnisoSampler, uvCoord + float2(0.0f, dx.y)).r);
	//float s3 = ((pos.z - SHADOW_EPSILON) < shadowMap[0].Sample(AnisoSampler, uvCoord + float2(dx.x, dx.y)).r);
	//return float4(s0, s1, s2, s3);

	//// Grid for seing shadowmap texels
	/*if (frac(texelPos.x) < 0.1 || frac(texelPos.y) < 0.1)
		return float4(0, 0.3, 0, 1);*/

	float4 test = float4(1, 1, 1, 1);
	for (int i = 0; i < nrOfKernels; i++) {
		test *= kernels[i];
	}

	//return clamp(float4(finalColor,1),0,1);
	//return viewPos;
	//return abs(noise);
	return test;
}