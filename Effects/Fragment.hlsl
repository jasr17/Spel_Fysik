struct GeoOut
{
	float4 PosW : POSITION;
	float4 PosH : SV_POSITION;
	float4 PosL : POSITION2;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
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
//cbuffer lightBuffer : register(b0)
//{
//    float4 lightCount;
//    float4 _lightPosition[10];
//    float4 _lightColor[10];
//};
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
Texture2D maps[3] : register(t0);
SamplerState samplerAni
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;
};

float4 PS_main(GeoOut input) : SV_Target
{
	float3 normal = normalize(input.Normal);
	float2 uv = input.TexCoord;
	float3 posW = input.PosW.xyz;
	//ambient
	float3 ambient = float3(0.1, 0.1, 0.1);
	//maps
	float3 map_ambient = mapUsages.x ? maps[0].Sample(samplerAni, uv) : float3(1,1,1);
	float3 map_diffuse = mapUsages.y ? maps[1].Sample(samplerAni, uv) : float3(1,1,1);
	float3 map_specular = mapUsages.z ? maps[2].Sample(samplerAni, uv) : float3(1,1,1);
	//resource color
	float3 textureColor = map_diffuse;
	float3 finalColor = textureColor * ambient * ambientReflectivity.rgb * map_ambient;
	for (int i = 0; i < lightCount.x; i++)
	{
		float3 lightPos = lights[i].position.xyz;
		float3 lightColor = lights[i].color.rgb;
		float lightIntensity = lights[i].color.a;

		//float3 lightPos = _lightPosition[i].xyz;
		//float3 lightColor = _lightColor[i].rgb;
		//float lightIntensity = _lightColor[i].a;

	float3 toLight = normalize(lightPos - posW);
	float dotNormaltoLight = dot(normal, toLight); //dot(normal, toLight) if less than one then the triangle is facing the other way, ignore
	if (dotNormaltoLight > 0)
	{
		//diffuse
		float distToLight = length(lightPos - posW);
		float diffuse = dotNormaltoLight;
		//specular
		float3 toCam = normalize(camPos.xyz - posW);
		float3 reflekt = normalize(2 * dotNormaltoLight * normal - toLight);
		float specular = pow(max(dot(reflekt, toCam), 0), specularReflectivity.w);

		finalColor += (diffuseReflectivity.rgb * textureColor * lightColor * diffuse * lightIntensity + textureColor * specular * specularReflectivity.rgb * map_specular) / pow(distToLight, 1.5);
	}
}
	//return shadowedTextureColor
	finalColor = clamp(finalColor, float3(0,0,0), float3(1, 1, 1));
	return float4(finalColor, 1);
}