struct VS_IN
{
	float3 Pos		: Position;
	float2 uv : TexCoordinate;
	//float3 Normal	: Normal;
};
struct VS_OUT
{
	float4 Pos			: SV_Position;
	float2 uv		: TEXCOORDS;
};

//PASSTHROUGH

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;
	output.Pos = float4(input.Pos,1);
	
	output.uv = input.uv;
	
	return output;
}