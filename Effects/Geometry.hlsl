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
    matrix mWorld,mView,mProj;
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
    float3x3 mWorld3x3 = mWorld;
    float3 up = float3(0,1,0);
    float3 rotUp = normalize(mul(up, mWorld3x3));
    for (int u = 0; u < 3; u++)
    {
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
        output[u].Normal = cross(rotUp, normalize(mul(cross(input[u].Normal,up), mWorld3x3))); //normal transformation in one swoop :)
        oStream.Append(output[u]);
    }
}