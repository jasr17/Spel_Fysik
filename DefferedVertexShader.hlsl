//konstanter

cbuffer Transforms {
	matrix WorldMatrix;
	matrix WorldVeiwMatrix;
	matrix WorldViewProjectionMatrix;

};


struct VSInput 
{
	float4 position : POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal	: NORMAL;
};

struct VSOutput {
	float4 positionCS	: SV_Position;
	float2 texCoord		: TEXCOORD;
	float3 normalWS		: NORMALWS;
	float3 positionWS	: POSITIONWS;
};

VSOutput VSMain(in VSInput input) {

	VSOutput output;

	//Convert position and normals to worldspace
	output.positionWS = mul(input.position, WorldMatrix).xyz;
	output.normalWS = normalize(mul(input.normal, (float3x3)WorldMatrix));

	output.texCoord = input.texCoord;

	return output;
}