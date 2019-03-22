// Konstanter

#define RADIUS 0.5
#define BIAS 0.0

const float2 NOISESCALE = float2(1920 / 8, 1080 / 8);


cbuffer matrixBuffer : register(b4)
{
	matrix mWorld, mInvTraWorld, mWorldViewPerspective;
	matrix mWorldViewMatrix, mProjectionMatrix;
}
cbuffer kernelBuffer : register(b3)
{
	float4 kernels[16];
	float4 nrOfKernels;
}

//Texturer/samplers
Texture2D Textures[5]		: register(t0);
Texture2D Noise				: register(t5);

SamplerState AnisoSampler;
SamplerState noiseSampler	: register(s0);


float3x3 SSAO_TBN(in float3 randomVector, in float3 normal)
{
	float3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
	float3 bitangent = cross(normal, tangent);
	return float3x3(tangent, bitangent, normal);
}

float occlusion(in float3x3 tbn, in float4 viewPos)
{
	float occlusion = 0.0;
	
	for (int i = 0; i < nrOfKernels.x; i++)
	{
		float3 currentSample = mul(kernels[i].xyz,tbn);
		currentSample = viewPos.xyz + currentSample * RADIUS;
		
		//Transform current sample to screen space and sample it.
		float4 sampleInScreenSpace = float4(currentSample, 1);
		sampleInScreenSpace = mul(sampleInScreenSpace, mProjectionMatrix);
		sampleInScreenSpace.xy /= sampleInScreenSpace.w;
		float2 uvCoord = float2(0.5f * sampleInScreenSpace.x + 0.5f, -0.5f * sampleInScreenSpace.y + 0.5f);
		float sampleDepth = Textures[4].Sample(AnisoSampler, uvCoord).z;

		float rangeCheck = abs(currentSample.z - sampleDepth) < RADIUS ? 1.0 : 0.5;
		occlusion += (sampleDepth < sampleInScreenSpace.z + BIAS ? 1.0 : 0.0)*rangeCheck;
	}
	occlusion =  1-(occlusion / nrOfKernels.x);

	return occlusion;
}

struct PS_IN
{
	float4 Pos	    : SV_Position;
	float2 uv		: TEXCOORDS;
};

struct PS_OUT
{
	float4 ssao		: SV_Target;
};

float4 PS_main(in PS_IN input) : SV_Target
{
	float4 normal = normalize(Textures[0].Sample(AnisoSampler, input.uv));
	float4 viewPos = Textures[4].Sample(AnisoSampler, input.uv);
	float4 randomVec = Noise.Sample(noiseSampler, input.uv*float2(240,135));


	
	float3x3 TBN = SSAO_TBN(randomVec, normal.xyz);

	PS_OUT o = (PS_OUT)0;
	float ao = occlusion(TBN, viewPos);
	
	return float4(ao, ao, ao, 1);
}