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
	void setConstantBuffer();
	
private:
	void generateKernelsAndNoise();
	//Kernels will function as samplepoints in a hemisphere above a wanted pixel
	float3 kernels[KERNELSIZE];
	//kernels will be rotated by a random value to lessen bandin effects.
	float3 noise[NOISESIZE*NOISESIZE];

	
	ID3D11Texture2D* noiseTexture = nullptr; 
	ID3D11ShaderResourceView* noiseShader = nullptr;
	ID3D11Buffer* cBuffer = nullptr;

	struct kernelConstantBuffer {
		float nrOfKernels;
		float3 kernels[KERNELSIZE];
		float pad1,pad2,pad3;
	};

};

