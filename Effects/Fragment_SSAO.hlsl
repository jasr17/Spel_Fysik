// Konstanter

#define RADIUS 0.05
#define BIAS 0.025
#define SCALE (1920/1080)

const float2 NOISESCALE = float2(1920 / 4, 1080 / 4);


cbuffer matrixBuffer : register(b4)
{
	matrix mWorld, mInvTraWorld, mWorldViewPerspective;
	matrix mWorldViewMatrix, mProjectionMatrix;
}
cbuffer kernelBuffer : register(b3)
{
	float4 kernels[64];
	float4 nrOfKernels;
}

//Texturer/samplers
Texture2D Textures[5]		: register(t0);
Texture2D Noise				: register(t5);

SamplerState AnisoSampler;
SamplerState noiseSampler	: register(s0);

//SSAO
//-----------------------------------------------------//
//Texture2D NoiseMap :register ();
//Först hitta pixelns position i viewspace och dess normal (textures2).



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
		float3 currentSample = mul(tbn, kernels[i].xyz);
		currentSample = viewPos.xyz + currentSample * RADIUS;

		float4 offSet = float4(currentSample, 1);
		offSet = mul(offSet, mProjectionMatrix);
		offSet.xy /= offSet.w;
		float2 uvCoord = float2(0.5f * offSet.x + 0.5f, -0.5f * offSet.y + 0.5f);

		float sampleDepth = Textures[4].Sample(AnisoSampler, uvCoord).z;
		float rangeCheck = abs(currentSample.z - sampleDepth) < RADIUS ? 1.0 : 0.0;
		occlusion += (sampleDepth >= offSet.z + BIAS ? 1.0 : 0.0);// *rangeCheck;
	}
	occlusion = 1 - (occlusion / nrOfKernels.x);

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
	float4 position = Textures[2].Sample(AnisoSampler, input.uv);
	float4 viewPos = Textures[4].Sample(AnisoSampler, input.uv);
	float4 randomVec = Noise.Sample(noiseSampler, input.uv * 64);


	
	float3x3 TBN = SSAO_TBN(randomVec.xyz, normal.xyz);

	PS_OUT o = (PS_OUT)0;
	float ao = occlusion(TBN, viewPos);
	
	//o.ssao = float4(ao,ao,ao,1);
	return float4(ao, ao, ao, 1);

}