// Konstanter
struct ShaderLight
{
	float4 position;
	float4 color; //.a is intensity
	float4x4 viewPerspectiveMatrix;
};
cbuffer lightBuffer : register(b0)
{
	float4 lightCount;
	ShaderLight lights[10];
};
cbuffer cameraBuffer : register(b1)
{
	float4 camPos;
}
cbuffer materialBuffer : register(b2)
{
	float4 ambientReflectivity;
	float4 diffuseReflectivity;
	float4 specularReflectivity;
	float4 mapUsages;
};



//Texturer/samplers

Texture2D Textures[3]	: register( t0 );
SamplerState AnisoSampler;

struct PixelShaderInput {
	float4 Pos	: SV_Position;
	float2 uv		: TEXCOORDS;
};


//G-bufferns pixelshader
float4 PS_main(in PixelShaderInput input) : SV_TARGET
{
	//float4 finalcolor;
float4 normal	= normalize(Textures[0].Sample(AnisoSampler, input.uv));
float4 position = Textures[2].Sample(AnisoSampler, input.uv);

float4 color	= Textures[1].Sample(AnisoSampler, input.uv);
float3 ambient = float3(0.1, .1, .1);
float3 finalColor = color.xyz;
for (int i = 0; i < lightCount.x; i++)
		{
			float3 lightPos = lights[i].position.xyz;
			float3 lightColor = lights[i].color.rgb;
			float lightIntensity = lights[i].color.a;
	
			
	
		float3 toLight = normalize(lightPos - position);
		float dotNormaltoLight = dot(normal.xyz, toLight); //dot(normal, toLight) if less than one then the triangle is facing the other way, ignore
		if (dotNormaltoLight > 0)
		{
			//diffuse
			float distToLight = length(lightPos - position);
			float diffuse = dotNormaltoLight;
			//specular
			float3 toCam = normalize(camPos.xyz - position);
			float3 reflekt = normalize(2 * dotNormaltoLight * normal - toLight);
			float specular = pow(max(dot(reflekt, toCam), 0), 50);
	
	           finalColor += lightIntensity * (diffuse * lightColor * color + diffuse * specular) / pow(distToLight, 0);
	        }
	}
	
	return clamp(float4(finalColor,1),0,1);
}