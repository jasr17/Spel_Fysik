struct PixelIn
{
    float4 PosW : POSITION;
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};
struct PixelShaderOutput
{
	float4 diffuse	: SV_Target0; //dbuffer
	float4 normal	: SV_Target1; //nbuffer
	float4 position	: SV_Target2; //pbuffer
};
c/*buffer lightBuffer : register(b0)
{
    float4 lightCount;
    float4 lightPos[10];
    float4 lightColor[10];
};
cbuffer cameraBuffer : register(b1)
{
    float4 camPos;
}*/

PixelShaderOutput PS_main(PixelIn ip) : SV_Target
{
	PixelShaderOutput op;
    op.normal = normalize(input.Normal);
    float2 uv = ip.TexCoord;
    op.position = ip.PosW.xyz;
    //ambient
    float3 ambient = float3(0.2, 0.2, 0.2);
    //resource color
    float3 textureColor = float3(1,1,1);
    float3 finalColor = textureColor * ambient;
    //for (int i = 0; i < lightCount.x; i++)
    //{
    //    float3 toLight = normalize(lightPos[i].xyz - posW);
    //    float dotNormaltoLight = dot(normal, toLight); //dot(normal, toLight) if less than one then the triangle is facing the other way, ignore
    //    if (dotNormaltoLight > 0)
    //    {
    //        //diffuse
    //        float distToLight = length(lightPos[i].xyz - posW);
    //        float diffuse = dotNormaltoLight;
    //        //specular
    //        float3 toCam = normalize(camPos.xyz - posW);
    //        float3 reflekt = normalize(2 * dotNormaltoLight * normal - toLight);
    //        float specular = pow(max(dot(reflekt, toCam), 0), 50);
    //        
    //        finalColor += (textureColor * lightColor[i].rgb * diffuse * lightColor[i].a + textureColor * specular) / pow(distToLight, 1);
    //    }
    //}
    //return shadowedTextureColor
    finalColor = clamp(finalColor, float3(0,0,0), float3(1, 1, 1));
	op.diffuse = finalColor;
    return op;
}