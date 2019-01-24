struct GeoOut
{
    float4 PosW : POSITION;
    float4 PosH : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float3 Normal : NORMAL;
};
cbuffer lightBuffer : register(b0)
{
    float4 lightPos;
    float4 lightColor;
};
cbuffer cameraBuffer : register(b2)
{
    float4 camPos;
}

SamplerState samplerAni
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;  
};

float4 PS_main(GeoOut input) : SV_Target
{
    float3 normal = input.Normal;
    //ambient
    float3 ambient = float3(0.0,0.0,0.0);
    //diffuse
    float distToLight = length(lightPos.xyz - input.PosW.xyz);
    float3 toLight = normalize(lightPos.xyz - input.PosW.xyz);
    float diffuse = clamp(dot(normal, toLight), 0, 1);
    //specular
    float3 toCam = normalize(camPos.xyz-input.PosW.xyz);
    float3 reflekt = normalize(2 * dot(normal, toLight) * normal - toLight);
    float specular = clamp(pow(clamp(dot(reflekt, toCam), 0, 1), 20),0,1);
    //resource color
    float heightDiffuse = clamp(input.PosW.y / 1, 0, 1);
    float3 green = float3(77.0f / 255, 168.0f / 255, 77.0f / 255);
    float3 brown = float3(104.0f / 255, 77.0f / 255, 42.0f / 255);
    float3 grey = float3(124.0f / 255, 124.0f / 255, 124.0f / 255);
    float3 white = float3(1, 1, 1);
    float slope = clamp(dot(float3(0, 1, 0), normal), 0, 1);
    float3 terrainColor;
    terrainColor = lerp(lerp(brown, grey, heightDiffuse), heightDiffuse > 0.4 ? lerp(green, white, sqrt((heightDiffuse - 0.4) / 0.6)) : green, pow(slope, 6));

    //return shadowedTextureColor
    float3 finalColor = terrainColor * ambient + (terrainColor * lightColor.rgb * diffuse * lightColor.a + terrainColor * specular) / pow(distToLight,0); //ambient, diffuse, no specular
    finalColor = min(finalColor, float3(1, 1, 1));
    return float4(finalColor, 1);
}