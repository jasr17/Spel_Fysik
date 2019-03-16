#pragma once
#include"standardClasses.h"
const int SSAO_TEXTURE_SLOT = 4;
const int KERNELSIZE = 10;
const int NOISESIZE = KERNELSIZE;
class SSAO
{
public:
	SSAO();
	virtual ~SSAO();
	void generateKernelsAndNoise();
	void setNosieTexture(ID3D11Device*);

	void setShaderSet(ShaderSet const &);
	
private:
	//Kernels will function as samplepoints in a hemisphere above a wanted pixel
	float3 kernels[KERNELSIZE];
	//kernels will be rotated by a random value to lessen bandin effects.
	float3 noise[NOISESIZE];

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Texture2D* noiseTexture = nullptr; 
	ID3D11RenderTargetView* noiseTarget = nullptr;
	ID3D11ShaderResourceView *noiseShader = nullptr;

	ShaderSet* shaderSet = nullptr;
};

