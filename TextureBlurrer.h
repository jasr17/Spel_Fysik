#pragma once
#include "standardClasses.h"
class TextureBlurrer
{
private:
	bool initilized = false;
	//shaders
	ID3D11ComputeShader* gCSH;
	ID3D11ComputeShader* gCSV;
	//out resource
	ID3D11Texture2D* gTexUAV;
	ID3D11UnorderedAccessView* gUAV;
	//in resource
	ID3D11Texture2D* gInTex;
	ID3D11ShaderResourceView* gInSRV;

	HRESULT createComputeShader(LPCWSTR filePath, ID3D11ComputeShader** computeShader);
	bool createResources(DXGI_FORMAT format);
	void release();
public:
	void blurTexture(ID3D11Resource* texture, int size_x, int size_y);
	bool initilize(DXGI_FORMAT format, LPCWSTR horizontalComputeFilePath, LPCWSTR verticalComputeFilePath);
	TextureBlurrer();
	~TextureBlurrer();
};
