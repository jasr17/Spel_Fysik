struct VS_IN
{
	float3 Pos : Position;
    float2 TexCoord : TexCoordinate;
    float3 Normal : Normal;
};
struct GeoOut
{
    float4 PosW : POSITION;
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};
cbuffer cbuffs : register(b0)
{
    matrix mWorld, mInvTraWorld, mWorldViewPerspective;
};

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
GeoOut VS_main(VS_IN input)
{
    GeoOut output = (GeoOut) 0;

    output.PosW = mul(float4(input.Pos, 1), mWorld);
    output.PosH = mul(float4(input.Pos, 1), mWorldViewPerspective);
    output.TexCoord = input.TexCoord;
    output.Normal = mul(float4(input.Normal, 0), mInvTraWorld).xyz;

    return output;
}