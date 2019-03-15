struct GeoOut
{
	float4 PosW : POSITION;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
cbuffer materialBuffer : register(b2)
{
	float4 ambientReflectivity;
	float4 diffuseReflectivity;
	float4 specularReflectivity;
	float4 mapUsages;
};
Texture2D maps[3] : register(t0);
SamplerState samplerAni
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
};

struct PS_Out
{
	float4 normal		:	SV_Target0;
	float4 albeno		:	SV_Target1;
	float4 position		:	SV_Target2;
    float4 specular     :   SV_TARGET3;
};


PS_Out PS_main(GeoOut ip)
{
	PS_Out op = (PS_Out)0;
	op.normal = float4(normalize(ip.Normal), 1);
    float3 map_diffuse = mapUsages.y ? maps[1].Sample(samplerAni, ip.TexCoord) : ambientReflectivity.xyz;
    float3 map_ambient = mapUsages.x ? maps[0].Sample(samplerAni, ip.TexCoord) : diffuseReflectivity.xyz;
    float3 map_specular = mapUsages.z ? maps[2].Sample(samplerAni, ip.TexCoord) : specularReflectivity.xyz;

	op.albeno = float4(map_diffuse*map_ambient,1);
	op.position = ip.PosW;
    op.specular = float4(map_specular, specularReflectivity.w);

	return op;
}