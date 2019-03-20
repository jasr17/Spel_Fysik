#include "LightManager.h"

void LightManager::releaseBuffers()
{
	if (lightBuffer != nullptr)lightBuffer->Release();
	if (matrixBuffer != nullptr)matrixBuffer->Release();
	for (int i = 0; i < maxLightCount; i++)
	{
		if (shadowMaps[i] != nullptr)shadowMaps[i]->Release();
		if (shaderResourceViewsDepth[i] != nullptr)shaderResourceViewsDepth[i]->Release();
	}
}

void LightManager::updateLightBuffer()
{
	gDeviceContext->UpdateSubresource(lightBuffer, 0, 0, &shadowMapLights, 0, 0);
}

void LightManager::updateMatrixBuffer(float4x4 worldMatrix, int index)
{
	float4x4 m = XMMatrixTranspose(XMMatrixMultiply(XMMatrixMultiply(worldMatrix, matrixViews[index]), matrixPerspective[index]));
	gDeviceContext->UpdateSubresource(matrixBuffer, 0, 0, &m, 0, 0);
}

void LightManager::bindShaderResourceDepthViews()
{
	gDeviceContext->PSSetShaderResources(10, lightCount(), shaderResourceViewsDepth);
}

void LightManager::bindLightBuffer()
{
	gDeviceContext->PSSetConstantBuffers(0, 1, &lightBuffer);
}

void LightManager::bindMatrixBuffer()
{
	gDeviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
}

float4x4 LightManager::getLightViewMatrix(int index) const
{
	return matrixViews[index];
}

void LightManager::setShadowMapForDrawing(int index)
{
	// Deactivates shader resource so it can be used as a render target view
	ID3D11ShaderResourceView* nullRTV = { NULL };
	gDeviceContext->PSSetShaderResources(0, 1, &nullRTV);
	//draw only to depth buffer, increased performance
	gDeviceContext->OMSetRenderTargets(0, NULL, shadowMaps[index]);

	gDeviceContext->ClearDepthStencilView(shadowMaps[index], D3D11_CLEAR_DEPTH, 1, 0);

	shader_shadowMap.bindShadersAndLayout();

	bindMatrixBuffer();
}

void LightManager::createShaderForShadowMap(LPCWSTR vertexName, LPCWSTR geometryName, LPCWSTR fragmentName, D3D11_INPUT_ELEMENT_DESC * inputDesc, int inputDescCount)
{
	shader_shadowMap.createShaders(vertexName, geometryName, fragmentName, inputDesc, inputDescCount);
}

void LightManager::createBuffers()
{
	releaseBuffers();
	//light buffer
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(ShaderLights);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &shadowMapLights;

	HRESULT hr = gDevice->CreateBuffer(&desc, &data, &lightBuffer);

	//matrixbuffer
	D3D11_BUFFER_DESC matdesc;
	memset(&matdesc, 0, sizeof(matdesc));
	// what type of buffer will this be?
	matdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	// what type of usage (press F1, read the docs)
	matdesc.Usage = D3D11_USAGE_DEFAULT;
	// how big in bytes each element in the buffer is.
	matdesc.ByteWidth = sizeof(float4x4);

	hr = gDevice->CreateBuffer(&matdesc, nullptr, &matrixBuffer);

	// Create texture space
	D3D11_TEXTURE2D_DESC texDesc;
	memset(&texDesc, 0, sizeof(texDesc));
	texDesc.Width = SMAP_WIDTH;
	texDesc.Height = SMAP_HEIGHT;
	texDesc.ArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	ID3D11Texture2D* pDepthBuffer = nullptr;

	// Create depth buffer object desc
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	memset(&dsvd, 0, sizeof(dsvd));
	dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	// Create a shader resource view desc
	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc;
	memset(&depthSRVDesc, 0, sizeof(depthSRVDesc));
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;
	depthSRVDesc.Texture2D.MipLevels = 1;

	for (int i = 0; i < lightCount(); i++)
	{
		hr = gDevice->CreateTexture2D(&texDesc, NULL, &pDepthBuffer);
		hr = gDevice->CreateDepthStencilView(pDepthBuffer, &dsvd, &shadowMaps[i]);
		hr = gDevice->CreateShaderResourceView(pDepthBuffer, &depthSRVDesc, &shaderResourceViewsDepth[i]);
		pDepthBuffer->Release();
	}
}

int LightManager::lightCount() const
{
	return (int)shadowMapLights.lightCount.x;
}

void LightManager::addLight(float3 position, float3 color, float intensity, float3 lookAt, float FOV, float nearPlane, float farPlane)
{
	ShaderLight* l = &shadowMapLights.lights[lightCount()];
	l->position = float4(position.x, position.y, position.z, 0);
	l->color = float4(color.x, color.y, color.z, intensity);
	float4x4 mv = XMMatrixLookAtLH(position, lookAt, float3(0, 1, 0));
	matrixViews[lightCount()] = mv;
	XMMATRIX perspective = XMMatrixPerspectiveFovLH(FOV, (float)(SMAP_WIDTH) / (SMAP_HEIGHT), nearPlane, farPlane);
	matrixPerspective[lightCount()] = perspective;
	l->viewPerspectiveMatrix = XMMatrixTranspose(XMMatrixMultiply(mv, perspective));
	shadowMapLights.lightCount.x++;
}

float3 LightManager::getLightPosition(int index)
{
	float3 p = float3(shadowMapLights.lights[index].position.x, shadowMapLights.lights[index].position.y, shadowMapLights.lights[index].position.z);
	return p;
}

LightManager::LightManager() {
	matrixViews.appendCapacity(maxLightCount);
	matrixPerspective.appendCapacity(maxLightCount);
	for (int i = 0; i < maxLightCount; i++)
	{
		shadowMaps[i] = nullptr;
		shaderResourceViewsDepth[i] = nullptr;
	}
}

LightManager::~LightManager()
{
	releaseBuffers();
}