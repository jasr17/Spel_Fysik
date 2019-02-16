struct VS_IN
{
	float3 Pos		: Position;
	float2 TexCoord : TexCoordinate;
	float3 Normal	: Normal;
};
struct VS_OUT
{
	float4 Pos		: SV_POSITION;
	float2 TexCoord : TEXCOORD;
};


//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	
	output.Pos = float4(input.Pos, 1);
	output.TexCoord = input.TexCoord;
	

	return output;
}