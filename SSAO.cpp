#include "SSAO.h"



SSAO::SSAO()
{
	generateKernelsAndNoise();
}


SSAO::~SSAO()
{
	if (noiseTexture != nullptr) {
		noiseTexture->Release();
	}
	if (noiseShader != nullptr) {
		noiseShader->Release();
	}
	if (cBuffer != nullptr) {
		cBuffer->Release();
	}
}

float lerp(float a, float b, float c) 
{

	return a + c * (b - a);
}

void SSAO::generateKernelsAndNoise()
{
	for (int i = 0; i < KERNELSIZE; i++) 
	{
		kernels[i] = float3(random(-1, 1), random(-1, 1), random(0, 1)); //Z will never be negative.
		kernels[i].Normalize();

		float scale = float(i) / float(KERNELSIZE);
		scale = lerp(0.1f, 1.0f, scale * scale);
		kernels[i] *= scale;
	}

	for (int i = 0; i < NOISESIZE*NOISESIZE; i++)//8x8 
	{
		noise[i] = float3(random(-1, 1), random(-1, 1),0); //rotation around Z
		noise[i].Normalize();
	}
}

void SSAO::createNosieTexture()
{
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	//8x8 texture
	td.Width = NOISESIZE;
	td.Height = NOISESIZE;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	td.SampleDesc.Count = 1;
	td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.CPUAccessFlags = 0;
	td.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = (void*)noise;
	data.SysMemPitch = NOISESIZE * sizeof(float3);

	gDevice->CreateTexture2D(&td, &data, &noiseTexture);

	

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = td.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	gDevice->CreateShaderResourceView(noiseTexture, &shaderResourceViewDesc, &noiseShader);
}

void SSAO::setNoise()
{
	gDeviceContext->PSSetShaderResources(SSAO_TEXTURE_SLOT, 1, &noiseShader);
}

void SSAO::createConstantBuffer()
{
	D3D11_BUFFER_DESC d;
	memset(&d, 0, sizeof(d));

	d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	d.Usage = D3D11_USAGE_DEFAULT;
	d.ByteWidth = sizeof(kernelConstantBuffer);

	kernelConstantBuffer cBuf;
	cBuf.nrOfKernels = KERNELSIZE;
	for (int i = 0; i < KERNELSIZE; i++)
	{
		cBuf.kernels[i] = kernels[i];
	}
	//D3D11_SUBRESOURCE_DATA data;
	//ZeroMemory(&data, sizeof(data));
	//data.pSysMem = &cBuf;

	gDevice->CreateBuffer(&d, nullptr, &cBuffer);
	gDeviceContext->PSSetConstantBuffers(2, 1, &cBuffer);

	gDeviceContext->UpdateSubresource(cBuffer, 0, 0, &cBuf, 0, 0);
}

