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
cbuffer materialBuffer : register(b1)
{
    float4 ambientReflectivty;
    float4 diffuseReflectivty;
    float4 specularReflectivty;
};
cbuffer cameraBuffer : register(b2)
{
    float4 camPos;
}

float4 PS_main(GeoOut input) : SV_Target
{
    float3 normal = input.Normal;
    //diffuse
    float distToLight = length(lightPos.xyz - input.PosW.xyz);
    float3 toLight = normalize(lightPos.xyz - input.PosW.xyz);
    float diffuse = dot(normal, toLight);
    //ambient
    float3 ambient = float3(0.0, 0.0, 0.0);
    //specular
    float3 toCam = normalize(camPos.xyz - input.PosW.xyz);
    float3 reflekt = normalize(2 * dot(normal, toLight) * normal - toLight);
    float specular = clamp(pow(clamp(dot(reflekt, toCam), 0, 1), specularReflectivty.w), 0, 1);
    //resource color
    //float3 textureColor = float3(1,1,1);//not using btw
    float3 textureColor = ambientReflectivty.rgb;

    //return shadowedTextureColor
    float3 finalColor = ambient * ambientReflectivty.rgb + (diffuseReflectivty.rgb * textureColor * lightColor.rgb * diffuse * lightColor.a + textureColor * specular * specularReflectivty.rgb) / pow(distToLight,0);
    finalColor = clamp(finalColor, float3(0,0,0), float3(1, 1, 1));
    return float4(finalColor, 1);
}