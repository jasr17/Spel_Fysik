#pragma once
#include"standardClasses.h"
const int SSAO_TEXTURE_SLOT = 5;
const int KERNELSIZE = 8;
const int NOISESIZE = 8;
class SSAO
{
public:
	SSAO();
	virtual ~SSAO();
	void createNosieTexture();
	void setNoise();
	void createConstantBuffer();
private:
	void generateKernelsAndNoise();
	//Kernels will function as samplepoints in a hemisphere above a wanted pixel
	float4 kernels[KERNELSIZE];
	//kernels will be rotated by a random value to lessen bandin effects.
	float3 noise[NOISESIZE*NOISESIZE];

	
	ID3D11Texture2D* noiseTexture = nullptr; 
	ID3D11ShaderResourceView* noiseShader = nullptr;
	ID3D11Buffer* cBuffer = nullptr;
	ID3D11SamplerState* noiseSampler = nullptr;

	struct kernelConstantBuffer {
		float4 kernels[KERNELSIZE];
		float4 nrOfKernels;
		
	};

};

