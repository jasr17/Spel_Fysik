
Texture2D<float4> gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

cbuffer buff : register(b0)
{
    float2 textureSize;
    float blurrSize;
}

float GaussianFunction(float x)
{
    float sigma = 1.f;
    float eUp1 = -(pow(x, 2.f));
    float eUp2 = 2 * pow(sigma, 2.f);
    float eUp = exp(eUp1 / eUp2);
    float base = 1.f / sqrt(2.f * 3.14159f * pow(sigma, 2.f));
    return base * eUp;
}

[numthreads(20, 20, 1)]

void main(uint3 DTid : SV_DispatchThreadID)
{
    float x = DTid.x, y = DTid.y;
    float scale = pow(length((float2(x, y) / textureSize) * 2 - 1) / sqrt(2), 2);
    float4 nPix = gInput.Load(int3(x, y, 0));

    int size = blurrSize;
    //set gaussian values
    float sum = 0;
    for (int l = 0; l < size; l++)
        sum += GaussianFunction(2 * ((float) l / (size - 1)) - 1);
    //apply values
    float4 pix = float4(0, 0, 0, 1);
    for (int xx = 0; xx < size; xx++)
       pix.xyz += GaussianFunction(2 * ((float) xx / (size - 1)) - 1) / sum * gInput.Load(int3(x - size / 2 + xx, y, 0)).xyz;

    gOutput[DTid.xy] = lerp(nPix,pix,scale);

}