// Konstanter
#define SHADOW_EPSILON 0.00001
#define SMAP_WIDTH  (1920 * 0.9)
#define SMAP_HEIGHT (1080 * 0.9)

struct ShaderLight
{
	float4 position;
	float4 color; //.a is intensity
	float4x4 viewPerspectiveMatrix;
};
cbuffer lightBuffer			: register(b0)
{
	float4 lightCount;
	ShaderLight lights[10];
};
cbuffer cameraBuffer		: register(b1)
{
	float4 camPos;
}
cbuffer matrixBuffer		: register(b4) {
	matrix mWorld, mInvTraWorld, mWorldViewPerspective;
	matrix mWorldViewMatrix, mProjectionMatrix;
}
cbuffer kernelBuffer		: register(b3) 
{
	float4 kernels[8];
	float4 nrOfKernels;
}

//Texturer/samplers
Texture2D Textures[5]		: register(t0);
Texture2D Noise				: register(t5);
Texture2D shadowMap[10]		: register(t10);
SamplerState AnisoSampler;
SamplerState noiseSampler	: register(s0);

//SSAO
//-----------------------------------------------------//
//Texture2D NoiseMap :register ();
//F�rst hitta pixelns position i viewspace och dess normal (textures2).



float3x3 SSAO_TBN(in float3 randomVector, in float3 normal) {
	float3 tangent = normalize(randomVector - normal * dot(randomVector, normal));
	float3 bitangent = cross(normal, tangent);
	return float3x3(tangent, bitangent, normal);
}

float3 getOrigin(in float z) {

	return camPos * z;
}

float occlusion(in float3x3 tbn,in float4 viewPos)
{
	float occlusion = 0.0;
	float4 view = viewPos;
	float check;
	for (int i = 0; i < nrOfKernels.x; i++) {
		view = float4(viewPos.xyz + mul(tbn, kernels[i]), 1);

		view = mul(view, mProjectionMatrix);
		view /= view.w;

		float2 uvCoord = float2(0.5f * view.x + 0.5f, -0.5f * view.y + 0.5f);
		float s0 = Textures[4].Sample(AnisoSampler, uvCoord).z;

		//rangeCheck
		 check = abs(view.z - s0) < 1 ? 1.0 : 0.0;
		check = 1;
		
		occlusion += (view.z < s0 ? 1.0 : 0.0) * check;
	}

	occlusion = 1 - (occlusion / nrOfKernels.x);
	return occlusion;
}



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
	float4 randomVec = Noise.Sample(noiseSampler, input.uv * 32);
	
	
	//OM KONSTIG BILD; BYT PLATS P� VIEWPOS OCH SSAO_TBN
	//float3x3 TBN = SSAO_TBN(normalize(randomVec), normal);				//	mul(normal.xyz, SSAO_TBN(normalize(randomVec), normal));
	float3x3 TBN = SSAO_TBN(normalize(float4(1,1,1,1)), normal);

	//return float4(viewPos.z, viewPos.z, viewPos.z,1);

	//-------------------------------------
	//SSAO
	//------------------------------------
	//To calculate the occlusion we need the pixels location in viewSpace
	float3 origin = getOrigin(position.x);

	//---------------------------------------
	float o = occlusion(TBN, viewPos);
	return float4(o, o, o, 1);
	float3 ambient = float3(0.2, 0.2, 0.2) * (occlusion(TBN, viewPos));
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
	
	float4 test = float4(1, 1, 1, 1);
	for (int i = 0; i < nrOfKernels.x; i++) {
		test.xyz *= kernels[i];
	}

	return clamp(float4(finalColor,1),0,1);
	//return float4(origin, 1);
	//return viewPos;
	//return abs(noise);
	/*if (nrOfKernels.x == 0)
		return float4(0, 0, 0, 1);*/
	//return viewPos;
	//return randomVec;
	//return float4(TBNVec, 1);
	
}