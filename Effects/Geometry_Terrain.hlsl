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
    matrix mWorld, mView, mProj;
};
SamplerState samplerAni;
Texture2D heightMap : register(t0);

[maxvertexcount(6)]
void GS_main(
	triangle VS_OUT input[3],
	inout TriangleStream< GeoOut > oStream
)
{
    //GeoOut points[3];
    //for (int i = 0; i < 3; i++)
    //{
    //    //input[i].Pos.y += heightMap.SampleLevel(samplerAni, input[i].TexCoord, 0).x;
    //    points[i].PosW = mul(input[i].Pos, mWorld);
    //    points[i].PosH = mul(input[i].Pos, mul(mul(mWorld, mView), mProj));
    //    points[i].TexCoord = input[i].TexCoord;

    //}
    //float3 normal = normalize(cross(points[1].PosW.xyz - points[0].PosW.xyz, points[2].PosW.xyz - points[0].PosW.xyz));
    //for (int j = 0; j < 3; j++)
    //{
    //    points[j].Normal = normal;
    //    oStream.Append(points[j]);
    //}

    GeoOut output[3];
    //matrices multiplication
    float3x3 mWorld3x3 = mWorld;
    float3 up = float3(0, 1, 0);
    float3 rotUp = normalize(mul(up, mWorld3x3));
    for (int u = 0; u < 3; u++)
    {
        input[u].Pos.y += heightMap.SampleLevel(samplerAni, input[u].TexCoord, 0).x;
        output[u].PosW = mul(input[u].Pos, mWorld);
        output[u].PosH = mul(input[u].Pos, mul(mul(mWorld, mView), mProj));
        output[u].TexCoord = input[u].TexCoord;
        /*
        Scaling the normal breaks it, but scaling the tangent doesnt!!, rotating breaks none of them. Dont translate them tho.
         * Retreive the tangent with cross on the normal and y-axis
         * Remove the translate on worldMatrix (make to a float3x3)
         * multiply tangent with matrix3x3, dont forget to do it with the y-axis as well!!
         * Retreive the normal again with cross on transformed_y-axis and tangent
        */
        output[u].Normal = cross(rotUp, normalize(mul(cross(input[u].Normal, up), mWorld3x3))); //normal transformation in one swoop :)
        oStream.Append(output[u]);
    }
}