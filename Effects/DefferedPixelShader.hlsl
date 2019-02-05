// Konstanter

cbuffer Transforms
{
	matrix WorldMatrix;
	matrix WorldVeiwMatrix;
	matrix WorldViewProjectionMatrix;

};


//Texturer/samplers

Texture2D DiffuseMap :register( t0 );
SamplerState AnisoSampler :register( s0 );

struct PixelShaderInput 
{
	float4 PositionScreenSpace	: SV_Position;
	float2 texCoord				: TEXCOORD;
	float3 normalWorldSpace		: NORMALWS;
	float3 positionWorldSpace	: POSITIONWS;
};
//Vi använder en Geometry-buffer för deffered shading
//G-buffern består av 3 buffrar, diffuse/normal/position - buffer.
struct PixelShaderOutput 
{
	float4 diffuse	:SV_Target0; //dbuffer
	float4 normal	:SV_Target1; //nbuffer
	float4 position	: SV_Target2; //pbuffer
};

//G-bufferns pixelshader
PixelShaderOutput PSMain(in PixelShaderInput input) 
{
	PixelShaderOutput output;


	//sample the diffusemap
	float3 diffuse = DiffuseMap.Sample(AnisoSampler, input.TexCoord).rbg;

	//Normalize the normal after interpolation
	float3 normalWS = normalize(input.normalWorldSpace);

	//Output G-buffervalues.
	output.normal = float4(normalWS, 1.0f);
	output.diffuse = float4(diffuse, 1.0f);
	output.position =(float4)
}