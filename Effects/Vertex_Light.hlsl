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

cbuffer cbuffs : register(b0)
{
	matrix mWorld, mInvTraWorld, mWorldViewPerspective, mLightWVP;
};


//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;
	// Render from the light's view
	output.Pos = mul(float4(input.Pos, 1), mLightWVP);
	return output;
}