#include "SSAO.h"



SSAO::SSAO()
{
}


SSAO::~SSAO()
{

}

float lerp(float a, float b, float c) {

	return a + c * (b - a);
}

void SSAO::generateKernelsAndNoise()
{
	for (int i = 0; i < KERNELSIZE; i++) {
		kernels[i] = float3(random(-1, 1), random(-1, 1), random(0, 1)); //Z will never be negative.
		kernels[i].Normalize();

		float scale = float(i) / float(KERNELSIZE);
		scale = lerp(0.1f, 1.0f, scale * scale);
		kernels[i] *= scale;
	}

	for (int i = 0; i < NOISESIZE; i++) {
		noise[i] = float3(random(-1, 1), random(-1, 1),0); //rotation around Z
		noise[i].Normalize();
	}
}

void SSAO::setNosieTexture(ID3D11Device *device)
{
	D3D11_BUFFER_DESC bufDesc;
	ZeroMemory(&bufDesc, sizeof(bufDesc));
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.Usage = D3D11_USAGE_DEFAULT;
	bufDesc.ByteWidth = sizeof(kernels);
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = kernels;

	device->CreateBuffer(&bufDesc, &data, &vertexBuffer);

	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));

	td.Width = Win_WIDTH;
	td.Height = Win_HEIGHT;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	td.SampleDesc.Count = 1;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	device->CreateTexture2D(&td, NULL, &noiseTexture);

	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
	renderTargetViewDesc.Format = td.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	device->CreateRenderTargetView(noiseTexture, &renderTargetViewDesc, &noiseTarget);


	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = td.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	device->CreateShaderResourceView(noiseTexture, &shaderResourceViewDesc, &noiseShader);

}

void SSAO::setShaderSet(ShaderSet const &item)
{
	shaderSet = new ShaderSet(item);
}
