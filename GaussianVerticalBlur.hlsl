
Texture2D<float4> gInput : register(t0);
RWTexture2D<float4> gOutput : register(u0);

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
void main( uint3 DTid : SV_DispatchThreadID )
{
    float x = DTid.x, y = DTid.y;
    float4 pix = float4(0, 0, 0, 1);

    const int size = 50;
    float gaussian[size];
    //set gaussian values
    float sum = 0;
    for (int l = 0; l < size; l++)
    {
        gaussian[l] = GaussianFunction(2 * ((float) l / (size - 1)) - 1);
        sum += gaussian[l];
    }
    //normalize values
    for (int l1 = 0; l1 < size; l1++)
        gaussian[l1] /= sum;
    //apply values
    for (int yy = 0; yy < size; yy++)
        pix.xyz += gaussian[yy] * gInput.Load(int3(x, y - size/2 + yy, 0)).xyz;

    gOutput[DTid.xy] = pix;

}