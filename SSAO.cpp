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
		kernels[i] = float4(random(-1.0, 1.0), random(-1.0, 1.0), random(0, 1), 0); //Z will never be negative.
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


	D3D11_SAMPLER_DESC sd;
	sd.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MipLODBias = 0;
	sd.MaxAnisotropy = 1;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	gDevice->CreateSamplerState(&sd, &noiseSampler);
}

void SSAO::setNoise()
{
	gDeviceContext->PSSetShaderResources(SSAO_TEXTURE_SLOT, 1, &noiseShader);
	gDeviceContext->PSSetSamplers(0, 1, &noiseSampler);
}

void SSAO::createConstantBuffer()
{
	D3D11_BUFFER_DESC d;
	memset(&d, 0, sizeof(d));

	d.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	d.Usage = D3D11_USAGE_DEFAULT;
	d.ByteWidth = sizeof(kernelConstantBuffer);

	kernelConstantBuffer cBuf;
	cBuf.nrOfKernels.x = KERNELSIZE;
	for (int i = 0; i < KERNELSIZE; i++)
	{
		cBuf.kernels[i] = float4(kernels[i]);
	}
	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = &cBuf;

	gDevice->CreateBuffer(&d, &data, &cBuffer);
	gDeviceContext->PSSetConstantBuffers(3, 1, &cBuffer);

	//gDeviceContext->UpdateSubresource(cBuffer, 0, 0, &cBuf, 0, 0);
}

