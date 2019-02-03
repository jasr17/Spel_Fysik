struct GeoOut
{
    float4 PosW : POSITION;
    float4 PosH : SV_POSITION;
	float4 PosL : POSITION;
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
cbuffer materialBuffer : register(b2)
{
    float4 ambientReflectivity;
    float4 diffuseReflectivity;
    float4 specularReflectivity;
};
Texture2D maps[3] : register(t0);
SamplerState samplerAni
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;
};

float4 PS_main(GeoOut input) : SV_Target
{
    float3 normal = input.Normal;
    float2 uv = input.TexCoord;
    float3 posW = input.PosW.xyz;
    //ambient
    float3 ambient = float3(0.1, 0.1, 0.1);
    //maps
    float3 map_ambient = maps[0].Sample(samplerAni, uv);
    float3 map_diffuse = maps[1].Sample(samplerAni, uv);
    float3 map_specular = maps[2].Sample(samplerAni, uv);
    //resource color
    float3 textureColor = map_diffuse;
    float3 finalColor = textureColor * ambient * ambientReflectivity.rgb;
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
            float specular = pow(max(dot(reflekt, toCam), 0), specularReflectivity.w);

            finalColor += (diffuseReflectivity.rgb * textureColor * lightColor[i].rgb * diffuse * lightColor[i].a + textureColor * specular * specularReflectivity.rgb) / pow(distToLight, 1);
        }
    }
    //return shadowedTextureColor
    finalColor = clamp(finalColor, float3(0,0,0), float3(1, 1, 1));
    return float4(finalColor, 1);
}