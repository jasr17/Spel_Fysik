struct PS_IN
{
	float4 PosW : POSITION;
	float4 PosH : SV_POSITION;
	float4 ViewPos : POSITION1;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
struct PS_OUT
{
	float4 normal		:	SV_Target0;
	float4 color		:	SV_Target1;
	float4 position		:	SV_Target2;
	float4 specular     :   SV_TARGET3;
	float4 viewPos		:	SV_TARGET4;
};
PS_OUT PS_main(PS_IN input) : SV_Target
{
	PS_OUT op = (PS_OUT)0;
    op.normal = float4(normalize(input.Normal),1);
	//terrain color
    float heightDiffuse = clamp(input.PosW.y / 5, 0, 1);
    float3 green = float3(77.0f / 255, 168.0f / 255, 77.0f / 255);
    float3 brown = float3(104.0f / 255, 77.0f / 255, 42.0f / 255);
    float3 grey = float3(124.0f / 255, 124.0f / 255, 124.0f / 255);
    float3 white = float3(1, 1, 1);
    float steepness = clamp(dot(float3(0, 1, 0), op.normal.xyz), 0, 1);
    //float3 terrainColor = lerp(lerp(brown, grey, heightDiffuse), heightDiffuse > 0.4 ? lerp(green, white, sqrt((heightDiffuse - 0.4) / 0.6)) : green, pow(slope, 20));
    float3 groundColor = lerp(green, white, pow(heightDiffuse,20));
    float3 wallColor = lerp(brown, grey, heightDiffuse);
    float3 terrainColor = lerp(wallColor, groundColor, pow(steepness,50));

    float3 finalColor = clamp(terrainColor, float3(0,0,0), float3(1,1,1));

	op.color = float4(finalColor, 1);
	op.position = input.PosW;
    op.specular = float4(1, 1, 1, 10);
	op.viewPos = input.ViewPos;
    return op;

}