struct PS_IN
{
	float4 PosW : POSITION;
	float4 PosH : SV_POSITION;
	float4 ViewPos : POSITION1;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
struct PS_OUT
{
	float4 normal		:	SV_Target0;
	float4 color		:	SV_Target1;
	float4 position		:	SV_Target2;
	float4 specular     :   SV_TARGET3;
	float4 viewPos		:	SV_TARGET4;
};

PS_OUT PS_main(PS_IN ip)
{
	PS_OUT op = (PS_OUT)0;
	op.normal = float4(normalize(ip.Normal), 1);
    op.color = float4(1,1,1,1);
	op.position = ip.PosW;
    op.specular = float4(0.5,0.5,0.5,2);
	op.viewPos = ip.ViewPos;

	return op;
}