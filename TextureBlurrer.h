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
public:
	void blurTexture(ID3D11Resource* texture, int size_x, int size_y);
	bool initilize(DXGI_FORMAT format, LPCWSTR horizontalComputeFilePath, LPCWSTR verticalComputeFilePath);
	TextureBlurrer();
	~TextureBlurrer();
};

/*THE INPUT RESOURCE HAS TO BE A TEXTURE2D AND COMPUTESHADER TEXTURE2D STUFF NEED TO BE THE SAME FORMAT AS INPUT TEXTURE*/
void TextureBlurrer::blurTexture(ID3D11Resource* texture, int size_x, int size_y) {
	if (initilized) {
		//fix dispatch size to cover the entire texture, DX, DY are the correct ones. 
		float dx = (float)size_x / 20, dy = (float)size_y / 20;
		int DX = dx + (dx - (int)dx > 0.01);
		int DY = dy + (dy - (int)dy > 0.01);
		//copy texture for input
		gDeviceContext->CopyResource(gInTex, texture);

		//set resources
		gDeviceContext->CSSetShaderResources(0, 1, &gInSRV);
		gDeviceContext->CSSetUnorderedAccessViews(0, 1, &gUAV, nullptr);
		//set to horizontal
		gDeviceContext->CSSetShader(gCSH, nullptr, 0);
		//dispatch horizontaly
		gDeviceContext->Dispatch(DX, DY, 1);
		//copy to old
		gDeviceContext->CopyResource(gInTex, gTexUAV);
		//set to vertical
		gDeviceContext->CSSetShader(gCSV, nullptr, 0);
		//dispatch verticaly
		gDeviceContext->Dispatch(DX, DY, 1);

		//copy back blurred texture
		gDeviceContext->CopyResource(texture, gTexUAV);
	}
}

inline bool TextureBlurrer::initilize(DXGI_FORMAT format, LPCWSTR horizontalComputeFilePath, LPCWSTR verticalComputeFilePath)
{
	if(!createResources(format))return false;
	if (FAILED(createComputeShader(horizontalComputeFilePath, &gCSH)))return false;
	if (FAILED(createComputeShader(verticalComputeFilePath, &gCSV)))return false;
	initilized = true;
	return true;
}

HRESULT TextureBlurrer::createComputeShader(LPCWSTR filePath, ID3D11ComputeShader** computeShader) {
	//create compute shader
	ID3DBlob* pCS = nullptr;
	ID3DBlob* errorBlob = nullptr;

	HRESULT result = D3DCompileFromFile(
		filePath,
		nullptr,
		nullptr,
		"main",
		"cs_5_0",
		D3DCOMPILE_DEBUG,
		0,
		&pCS,
		&errorBlob
	);
	// compilation failed?
	if (FAILED(result))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			// release "reference" to errorBlob interface object
			errorBlob->Release();
		}
		if (pCS)
			pCS->Release();
		return result;
	}
	result = gDevice->CreateComputeShader(pCS->GetBufferPointer(), pCS->GetBufferSize(), 0, computeShader);
	return result;
}

inline bool TextureBlurrer::createResources(DXGI_FORMAT format)
{
	//texture input
	D3D11_TEXTURE2D_DESC inTexDesc;
	ZeroMemory(&inTexDesc, sizeof(inTexDesc));
	inTexDesc.Width = Win_WIDTH;
	inTexDesc.Height = Win_HEIGHT;
	inTexDesc.MipLevels = 1;
	inTexDesc.ArraySize = 1;
	inTexDesc.Format = format;//DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R8G8B8A8_UNORM;
	inTexDesc.SampleDesc.Count = 1;
	inTexDesc.SampleDesc.Quality = 0;
	inTexDesc.Usage = D3D11_USAGE_DEFAULT;
	inTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	inTexDesc.CPUAccessFlags = 0;
	inTexDesc.MiscFlags = 0;

	HRESULT hr = gDevice->CreateTexture2D(&inTexDesc, nullptr, &gInTex);
	//view input
	D3D11_SHADER_RESOURCE_VIEW_DESC inSRVDesc;
	ZeroMemory(&inSRVDesc, sizeof(inSRVDesc));
	inSRVDesc.Format = inTexDesc.Format;
	inSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	inSRVDesc.Texture2D.MostDetailedMip = 0;
	inSRVDesc.Texture2D.MipLevels = 1;

	hr = gDevice->CreateShaderResourceView(gInTex, &inSRVDesc, &gInSRV);

	//create 2d output texture
	D3D11_TEXTURE2D_DESC blurredTexDesc;
	ZeroMemory(&blurredTexDesc, sizeof(blurredTexDesc));
	blurredTexDesc.Width = Win_WIDTH;
	blurredTexDesc.Height = Win_HEIGHT;
	blurredTexDesc.MipLevels = 1;
	blurredTexDesc.ArraySize = 1;
	blurredTexDesc.Format = format;//DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_R8G8B8A8_UNORM;
	blurredTexDesc.SampleDesc.Count = 1;
	blurredTexDesc.SampleDesc.Quality = 0;
	blurredTexDesc.Usage = D3D11_USAGE_DEFAULT;
	blurredTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	blurredTexDesc.CPUAccessFlags = 0;
	blurredTexDesc.MiscFlags = 0;
	if (FAILED(gDevice->CreateTexture2D(&blurredTexDesc, nullptr, &gTexUAV))) return false;

	//create access view output
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = blurredTexDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	if (FAILED(gDevice->CreateUnorderedAccessView(gTexUAV, &uavDesc, &gUAV)))return false;

	//create shader
	if (FAILED(createComputeShader(L"GaussianHorizontalBlur.hlsl", &gCSH))) return false;
	if (FAILED(createComputeShader(L"GaussianVerticalBlur.hlsl", &gCSV))) return false;

	return true;
}

inline TextureBlurrer::TextureBlurrer() {

}

inline TextureBlurrer::~TextureBlurrer()
{
}
