struct VS_IN
{
	float3 Pos : Position;
	float2 TexCoord : TexCoordinate;
	float3 Normal : Normal;
};
struct VS_OUT
{
	float4 Pos : SV_POSITION;
};

cbuffer CB : register(b0)
{
	float4x4 worldMat;
}


//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	// Render from the light's view
	return mul(worldMat, float4(input.Pos, 1));
}