struct GeoOut
{
    float4 PosW : POSITION;
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};
cbuffer lightBuffer : register(b0)
{
    float4 lightCount;
    float4 lightPos[10];
    float4 lightColor[10];
};
cbuffer cameraBuffer : register(b1)
{
    float4 camPos;
}

cbuffer matrixBuffer : register(b2)
{
	matrix mWorld, mInvTraWorld, mView, mProj, mLightWVP;
}

float4 PS_main(GeoOut input) : SV_Target
{
    float3 normal = input.Normal;
    float2 uv = input.TexCoord;
    float3 posW = input.PosW.xyz;
    //ambient
    float3 ambient = float3(0.1, 0.1, 0.1);
    //resource color
    float3 textureColor = float3(1,1,1);
    float3 finalColor = textureColor * ambient;
    for (int i = 0; i < lightCount.x; i++)
    {
        //diffuse
        float distToLight = length(lightPos[i].xyz - posW);
        float3 toLight = normalize(lightPos[i].xyz - posW);
        float diffuse = max(dot(normal, toLight),0);
        //specular
        float3 toCam = normalize(camPos.xyz - input.PosW.xyz);
        float3 reflekt = normalize(2 * dot(normal, toLight) * normal - toLight);
        float specular = pow(max(dot(reflekt, toCam), 0), 1);

        finalColor += (textureColor * lightColor[i].rgb * diffuse * lightColor[i].a + textureColor * specular) / pow(distToLight, 3);
    }
    //return shadowedTextureColor
    finalColor = clamp(finalColor, float3(0,0,0), float3(1, 1, 1));
    return float4(finalColor, 1);
}