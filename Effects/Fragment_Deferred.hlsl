// Konstanter

cbuffer lightBuffer : register(b0) {
	float4 lightcount;
	float4 lightPos[10];
	float4 lightColor[10];
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
//Vi använder en Geometry-buffer för deferred shading
//G-buffern består av 3 buffrar, diffuse/normal/position - buffer.
struct PixelShaderOutput 
{
	float4 diffuse	:SV_Target0; //dbuffer
	float4 normal	:SV_Target1; //nbuffer
	float4 position	: SV_Target2; //pbuffer
};

//G-bufferns pixelshader
PixelShaderOutput PSMain(in PixelShaderInput input) : SV_Target
{
	PixelShaderOutput output;


	//sample the diffusemap
	float3 diffuse = DiffuseMap.Sample(AnisoSampler, input.TexCoord).rbg;

	//Normalize the normal after interpolation
	float3 normalWS = normalize(input.normalWorldSpace);

	for (int i = 0; i < lightCount.x; i++)
	{
		float3 toLight = normalize(lightPos[i].xyz - posW);
		float dotNormaltoLight = dot(normal, toLight); //dot(normal, toLight) if less than one then the triangle is facing the other way, ignore
		if (dotNormaltoLight > 0)
		{
			//diffuse
			float distToLight = length(lightPos[i].xyz - posW);
			float diffuse = dotNormaltoLight;
			//specular
			float3 toCam = normalize(camPos.xyz - posW);
			float3 reflekt = normalize(2 * dotNormaltoLight * normal - toLight);
			float specular = pow(max(dot(reflekt, toCam), 0), 50);

			finalColor += (textureColor * lightColor[i].rgb * diffuse * lightColor[i].a + textureColor * specular) / pow(distToLight, 1);
		}
	}

	//Output G-buffervalues.
	output.normal = float4(normalWS, 1.0f);
	output.diffuse = float4(diffuse, 1.0f);
	output.position = float4(input.Position, 1.0f);

	return output;
}