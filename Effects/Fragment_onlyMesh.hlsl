struct GeoOut
{
	float4 PosW : POSITION;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
struct PS_Out
{
	float4 normal		:	SV_Target0;
	float4 diffuse		:	SV_Target1;
	float4 position		:	SV_Target2;
    float4 specular     :   SV_TARGET3;
};

PS_Out PS_main(GeoOut ip)
{
	PS_Out op = (PS_Out)0;
	op.normal = float4(normalize(ip.Normal), 1);
    op.diffuse = float4(1,1,1,1);
	op.position = ip.PosW;
    op.specular = float4(0.5,0.5,0.5,2);

	return op;
}