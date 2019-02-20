Texture2D shadowMap[10] : register(t0);
SamplerState mySampler;

struct GeoOut
{
	float4 PosW : POSITION;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
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
struct PS_Out
{
	float4 normal		:	SV_Target0;
	float4 color		:	SV_Target1;
	float4 position		:	SV_Target2;
};

bool checkShadowMap(float4 pos, int shadowMapIndex)
{
	// Manually devides by w
	pos.xyz /= pos.w;
	// sets coordinates from a [-1, 1] range to [0, 1]. Flips y since directx 0 starts on top
	float2 uvCoord = float2(0.5f*pos.x + 0.5f, -0.5f*pos.y + 0.5f);

	// Compares (depth - bias) with the shadowmap
    return (pos.z - 0.00001 < shadowMap[shadowMapIndex].Sample(mySampler, uvCoord).r);
}
//
//PS_Out PS_main(GeoOut ip)
//{
//	PS_Out op = (PS_Out)0;
//	op.normal = float4(normalize(ip.Normal),1);
//	op.diffuse = (0, 1, 0, 1);
//	op.position = ip.PosH;
//
//	return op;
//}


//OLD OUTPUT

PS_Out PS_main(GeoOut input) : SV_Target
{
	PS_Out op = (PS_Out)0;
    op.normal = float4(normalize(input.Normal),1);
	//ambient
    //float3 ambient = float3(0.1, 0.1, 0.1);
	//resource color
    float heightDiffuse = clamp(input.PosW.y / 5, 0, 1);
    float3 green = float3(77.0f / 255, 168.0f / 255, 77.0f / 255);
    float3 brown = float3(104.0f / 255, 77.0f / 255, 42.0f / 255);
    float3 grey = float3(124.0f / 255, 124.0f / 255, 124.0f / 255);
    float3 white = float3(1, 1, 1);
    float slope = clamp(dot(float3(0, 1, 0), input.Normal), 0, 1);
    float3 terrainColor = lerp(lerp(brown, grey, heightDiffuse), heightDiffuse > 0.4 ? lerp(green, white, sqrt((heightDiffuse - 0.4) / 0.6)) : green, pow(slope, 20));

    float3 finalColor = terrainColor;
    //for (int i = 0; i < lightCount.x; i++)
    //{
    //    if (checkShadowMap(mul(float4(input.PosW.xyz, 1), lights[i].viewPerspectiveMatrix), i))
    //    {

    //        float3 lightPos = lights[i].position.xyz;
    //        float3 lightColor = lights[i].color.rgb;
    //        float lightIntensity = lights[i].color.a;

    //        float3 toLight = normalize(lightPos - input.PosW.xyz);
    //        float dotNormaltoLight = dot(normal, toLight); //dot(normal, toLight) if less than one then the triangle is facing the other way, ignore
    //        if (dotNormaltoLight > 0)
    //        {
				//	//diffuse
    //            float distToLight = length(lightPos - input.PosW.xyz);
    //            float diffuse = dotNormaltoLight;
				//	//specular
    //            float3 toCam = normalize(camPos.xyz - input.PosW.xyz);
    //            float3 reflekt = normalize(2 * dotNormaltoLight * normal - toLight);
    //            float specular = pow(max(dot(reflekt, toCam), 0), 50);

    //        }
    //    }
    //}
    //return shadowedTextureColor
	finalColor = clamp(finalColor,0,1);
	op.color = float4(finalColor, 1); //clamp(finalColor, float3(0, 0, 0), float3(1, 1, 1)),1);
	op.position = input.PosH;
    return op;

}