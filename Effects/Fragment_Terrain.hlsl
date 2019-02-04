Texture2D shadowMap : register(t0);
SamplerState mySampler;

struct GeoOut
{
    float4 PosW : POSITION;
    float4 PosH : SV_POSITION;
	float4 PosL : POSITION2;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};
cbuffer lightBuffer : register(b0)
{
    float4 lightCount;
    float4 lightPos[10];
    float4 lightColor[10];
};
cbuffer cameraBuffer : register(b1)
{
    float4 camPos;
}

bool checkShadowMap(float4 pos)
{
	// Manually devides by w
	pos.xyz /= pos.w;
	// sets coordinates from a [-1, 1] range to [0, 1]. Flips y since directx 0 starts on top
	float2 uvCoord = float2(0.5f*pos.x + 0.5f, -0.5f*pos.y + 0.5f);
	
	// Compares (depth - bias) with the shadowmap
	return pos.z - 0.00001 < shadowMap.Sample(mySampler, uvCoord).r;
}

float4 PS_main(GeoOut input) : SV_Target
{
    float3 normal = input.Normal;
    //ambient
    float3 ambient = float3(0.1,0.1,0.1);
    //resource color
    float heightDiffuse = clamp(input.PosW.y / 5, 0, 1);
    float3 green = float3(77.0f / 255, 168.0f / 255, 77.0f / 255);
    float3 brown = float3(104.0f / 255, 77.0f / 255, 42.0f / 255);
    float3 grey = float3(124.0f / 255, 124.0f / 255, 124.0f / 255);
    float3 white = float3(1, 1, 1);
    float slope = clamp(dot(float3(0, 1, 0), normal), 0, 1);
    float3 terrainColor = lerp(lerp(brown, grey, heightDiffuse), heightDiffuse > 0.4 ? lerp(green, white, sqrt((heightDiffuse - 0.4) / 0.6)) : green, pow(slope, 20));

    float3 finalColor = terrainColor*ambient;
	if (checkShadowMap(input.PosL))
		for (int i = 0; i < lightCount.x; i++)
		{
		    float3 toLight = normalize(lightPos[i].xyz - input.PosW.xyz);
		    float dotNormaltoLight = dot(normal, toLight); //dot(normal, toLight) if less than one then the triangle is facing the other way, ignore
		    if (dotNormaltoLight > 0)
		    {
		        //diffuse
		        float distToLight = length(lightPos[i].xyz - input.PosW.xyz);
				float diffuse = dotNormaltoLight;// *checkShadowMap(input.PosL);
		        //specular
		        float3 toCam = normalize(camPos.xyz - input.PosW.xyz);
		        float3 reflekt = normalize(2 * dotNormaltoLight * normal - toLight);
		        float specular = pow(max(dot(reflekt, toCam), 0), 50);
				
				finalColor += (terrainColor * lightColor[i].rgb * diffuse * lightColor[i].a + terrainColor * specular) / pow(distToLight, 1);
		    }
		}

    //return shadowedTextureColor
    finalColor = clamp(finalColor, float3(0, 0, 0), float3(1, 1, 1));
    return float4(finalColor, 1);
}