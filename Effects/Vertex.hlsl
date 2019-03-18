struct VS_IN
{
	float3 Pos : Position;
    float2 TexCoord : TexCoordinate;
    float3 Normal : Normal;
};
struct VS_OUT
{
    float4 PosW : POSITION;
    float4 PosH : SV_POSITION;
	float4 ViewPos : POSITION1;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};
cbuffer cbuffs : register(b0)
{
    matrix mWorld, mInvTraWorld, mWorldViewPerspective;
	matrix mWorldViewMatrix, mProjectionMatrix;
};

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
    VS_OUT output = (VS_OUT) 0;

    output.PosW = mul(float4(input.Pos, 1), mWorld);
    output.PosH = mul(float4(input.Pos, 1), mWorldViewPerspective);
    output.TexCoord = input.TexCoord;
    output.Normal = mul(float4(input.Normal, 0), mInvTraWorld).xyz;
	output.ViewPos = mul(float4(input.Pos, 1), mWorldViewMatrix);

    return output;
}