struct VS_OUT
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};
struct GeoOut
{
    float4 PosW : POSITION;
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};
cbuffer cbuffs
{
    matrix mWorld,mInvTraWorld,mView,mProj;
};

[maxvertexcount(3)]
void GS_main(
	triangle VS_OUT input[3], 
	inout TriangleStream< GeoOut > oStream
)
{
    //GeoOut output[3];
    //matrices multiplication
    //for (int u = 0; u < 3; u++)
    //{
    //    output[u].PosW = mul(input[u].Pos, mWorld);
    //    output[u].PosH = mul(input[u].Pos, mul(mul(mWorld, mView), mProj));
    //    float3 temp = input[u].Normal;
    //}
    //get normal
    //float3 normal = normalize(cross(output[1].PosW.xyz - output[0].PosW.xyz, output[2].PosW.xyz - output[0].PosW.xyz));
    //pass on texcoord, normal and append first triangle
    //for (int i = 0; i < 3; i++)
    //{
    //    output[i].TexCoord = input[i].TexCoord;
    //    output[i].Normal = normal;
    //    oStream.Append(output[i]);
    //}

    GeoOut output[3];
    //matrices multiplication
    float3x3 mInvTraWorld3x3 = mInvTraWorld;
    for (int u = 0; u < 3; u++)
    {
        output[u].PosW = mul(input[u].Pos, mWorld);
        output[u].PosH = mul(input[u].Pos, mul(mul(mWorld, mView), mProj));
        output[u].TexCoord = input[u].TexCoord;
        output[u].Normal = normalize(mul(input[u].Normal, mInvTraWorld3x3));
        oStream.Append(output[u]);
    }
}