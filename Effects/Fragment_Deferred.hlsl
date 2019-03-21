// Konstanter
#define SHADOW_EPSILON (0.000008)

struct ShaderLight
{
	float4 position;
	float4 color; //.a is intensity
	float4x4 viewPerspectiveMatrix;
	float4 smapSize;
};
cbuffer lightBuffer			: register(b0)
{
	float4 lightCount;
	ShaderLight lights[10];
};
cbuffer cameraBuffer		: register(b1)
{
	float4 camPos;
};
cbuffer showcaseDeferredMaps : register(b6)
{
	float4 showcaseMap; // only uses x
};

//Texturer/samplers
Texture2D Textures[5]		: register(t0);
Texture2D SSAO				: register(t6);
Texture2D shadowMap[10]		: register(t10);
SamplerState AnisoSampler;
SamplerState noiseSampler	: register(s0);


float checkShadowMap(float4 pos, int shadowMapIndex)
{
	// Manually devides by w
	pos.xyz /= pos.w;
	float depth = pos.z;
	// sets coordinates from a [-1, 1] range to [0, 1]. Flips y since directx 0 starts on top
	float2 uvCoord = float2(0.5f * pos.x + 0.5f, -0.5f * pos.y + 0.5f);

	float2 mapSize = float2(lights[shadowMapIndex].smapSize.x, lights[shadowMapIndex].smapSize.y);
	float2 dx = float2(1.0f / lights[shadowMapIndex].smapSize.x, 1.0f / lights[shadowMapIndex].smapSize.y); // size of one texel
	
	float2 texelPos = uvCoord * mapSize; 
	float2 lerps = frac(texelPos); // Position on the texel. Used in linear interpolation to judge weight of contributon
	
	// Aligns uvCoords to corner of texel so lerps weights make sense
	uvCoord = (texelPos - lerps) / mapSize;

	// Compares (depth - bias) with the shadowmap on 4 neighburing texels
	float s0 = ((depth - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord).r);
	float s1 = ((depth - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, 0.0f)).r);
	float s2 = ((depth - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(0.0f, dx.y)).r);
	float s3 = ((depth - SHADOW_EPSILON) < shadowMap[shadowMapIndex].Sample(AnisoSampler, uvCoord + float2(dx.x, dx.y)).r);
	
	// Interpolates the result of the sampling
	float shadowCoeff = lerp( lerp(s0, s1, lerps.x), lerp(s2, s3, lerps.x), lerps.y);

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
	float ssao = SSAO.Sample(AnisoSampler, input.uv).x;

	// Used to showcase the deferred maps
	if (showcaseMap.x == 2) return normal;
	if (showcaseMap.x == 3) return color;
	if (showcaseMap.x == 4) return position;
	if (showcaseMap.x == 5) return specular;
	if (showcaseMap.x == 6) return ssao;
	if (showcaseMap.x == 7)
	{
		if (input.uv.x < 0.5 && input.uv.y < 0.5)
			return normalize(Textures[0].Sample(noiseSampler, input.uv * 2));
		if (input.uv.x > 0.5 && input.uv.y < 0.5)
			return Textures[1].Sample(noiseSampler, input.uv * 2);
		if (input.uv.x < 0.5 && input.uv.y > 0.5)
			return Textures[2].Sample(noiseSampler, input.uv * 2);
		if (input.uv.x > 0.5 && input.uv.y > 0.5)
			return Textures[3].Sample(noiseSampler, input.uv * 2);
	}

	float3 ambient = float3(0.2, 0.2, 0.2);
    float3 finalColor = color.xyz * ambient *ssao;
	for (int i = 0; i < lightCount.x; i++)
    {
		float shadowCoeff = checkShadowMap(mul(position, lights[i].viewPerspectiveMatrix), i);		
        if (shadowCoeff > 0)
        {
            float3 lightPos = lights[i].position.xyz;
            float3 lightColor = lights[i].color.rgb;
            float lightIntensity = lights[i].color.a;

            float3 toLight = normalize(lightPos - position.xyz);
            float dotNormaltoLight = dot(normal.xyz, toLight); 
            if (dotNormaltoLight > 0)
            {
			    //diffuse
                float distToLight = length(lightPos - position.xyz);
                float diffuseStrength = dotNormaltoLight;
			   	//specular
                float3 toCam = normalize(camPos.xyz - position.xyz);
                float3 reflekt = normalize(2 * dotNormaltoLight * normal.xyz - toLight);
                float specularStrength = pow(max(dot(reflekt, toCam), 0), specular.w);
								
				finalColor += (lightIntensity * (color.xyz * lightColor * diffuseStrength + color.xyz * specularStrength * specular.rgb) / pow(distToLight, 0)) * shadowCoeff;
            }
        }
    }
	return clamp(float4(finalColor,1),0,1);	
}