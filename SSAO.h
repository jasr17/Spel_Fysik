#pragma once
#include"TextureBlurrer.h"
#include"standardClasses.h"
const int SSAO_TEXTURE_SLOT = 5;
const int KERNELSIZE = 16;
const int NOISESIZE = 8;
class SSAO
{
private:

	ID3D11RenderTargetView* SSAOTarget = nullptr;
	ID3D11Texture2D* SSAOTexture = nullptr;
	ID3D11ShaderResourceView* SSAOShaderResource = nullptr;

	ShaderSet *shaderSet;
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

	TextureBlurrer blurrer;

public:
	SSAO();
	virtual ~SSAO();
	void createNosieTexture();
	void setNoise();
	void createConstantBuffer();
	void setShaderSet(ShaderSet const&);
	void createSSAOShaderResources();
	void setPS();
	void setOMRenderTarget(ID3D11RenderTargetView * backBuffer);
	void addBlurr();
	ShaderSet* getShader()const;
	ID3D11ShaderResourceView* getSRV()const;
	ID3D11RenderTargetView* getTargetView()const;

};

