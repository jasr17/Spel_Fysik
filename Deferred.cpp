#include "Deferred.h"


Deferred::Deferred()
{
	
}

void Deferred::initDeferred(ID3D11Device * device)
{
	FSQ.CreateQuad(device);
	CreateGBuffer(device);
}


bool Deferred::CreateGBuffer(ID3D11Device * device) // Om denna flyttas till en egen klass senare behöver den tillgång till Device och skärm info(width,height,depth
	{
		HRESULT result;

		// Initialize the render target texture description.
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the render target texture description.
		textureDesc.Width = Win_WIDTH;
		textureDesc.Height = Win_HEIGHT;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the render target textures.
		for (int i = 0; i < DEFERRED_BUFFERCOUNT; i++)
		{
			result = device->CreateTexture2D(&textureDesc, NULL, &gBuffer[i].texture);
			if (FAILED(result))
			{
				return false;
			}
		}

		// Setup the description of the render target view.
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		ZeroMemory(&renderTargetViewDesc, sizeof(renderTargetViewDesc));
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create the render target views.
		for (int i = 0; i < DEFERRED_BUFFERCOUNT; i++)
		{
			result = device->CreateRenderTargetView(gBuffer[i].texture, &renderTargetViewDesc, &gBuffer[i].renderTargetView);
			if (FAILED(result))
			{
				return false;
			}
		}

		// Setup the description of the shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
		ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create the shader resource views.
		for (int i = 0; i < DEFERRED_BUFFERCOUNT; i++)
		{
			result =device->CreateShaderResourceView(gBuffer[i].texture, &shaderResourceViewDesc, &gBuffer[i].shaderResourceView);
			if (FAILED(result))
			{
				return false;
			}
		}

		//// Initialize the description of the depth buffer.
		//D3D11_TEXTURE2D_DESC depthBufferDesc;
		//ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		//// Set up the description of the depth buffer.
		//depthBufferDesc.Width = Win_WIDTH;
		//depthBufferDesc.Height = Win_HEIGHT;
		//depthBufferDesc.MipLevels = 1;
		//depthBufferDesc.ArraySize = 1;
		//depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//depthBufferDesc.SampleDesc.Count = 1;
		//depthBufferDesc.SampleDesc.Quality = 0;
		//depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		//depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		//depthBufferDesc.CPUAccessFlags = 0;
		//depthBufferDesc.MiscFlags = 0;

		//// Create the texture for the depth buffer using the filled out description.
		//result = gDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
		//if (FAILED(result))
		//{
		//	return false;
		//}

		//// Initailze the depth stencil view description.
		//ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		//// Set up the depth stencil view description.
		//D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		//depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		//depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		//depthStencilViewDesc.Texture2D.MipSlice = 0;

		//// Create the depth stencil view.
		//result = device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
		//if (FAILED(result))
		//{
		//	return false;
		//}

		//// Setup the viewport for rendering.
		//m_viewport.Width = (float)textureWidth;
		//m_viewport.Height = (float)textureHeight;
		//m_viewport.MinDepth = 0.0f;
		//m_viewport.MaxDepth = 1.0f;
		//m_viewport.TopLeftX = 0.0f;
		//m_viewport.TopLeftY = 0.0f;

		return true;
}

Deferred::~Deferred()
{
	delete shaderSet;
	for (int i = 0; i < DEFERRED_BUFFERCOUNT; i++) {
		if (gBuffer[i].renderTargetView != nullptr) gBuffer[i].renderTargetView->Release();
		if (gBuffer[i].texture != nullptr)gBuffer[i].texture->Release();
		if (gBuffer[i].shaderResourceView != nullptr) gBuffer[i].shaderResourceView->Release();
	}
}

ShaderSet Deferred::getShaderSet()
{
	return *shaderSet;
}

void Deferred::setShaderSet(ShaderSet const &item)
{
	shaderSet = new ShaderSet(item);
}

void Deferred::BindFirstPass(ID3D11DeviceContext* context,ID3D11DepthStencilView* zBuffer)
{
	ID3D11RenderTargetView* renderTargets[DEFERRED_BUFFERCOUNT];
	for (int i = 0; i < DEFERRED_BUFFERCOUNT; i++) {
		renderTargets[i] = gBuffer[i].renderTargetView;
	}
	context->OMSetRenderTargets(DEFERRED_BUFFERCOUNT, renderTargets, zBuffer);

	//clear rendertargets
	float color[] = { 0,1,0,1.f };
	for (int i = 0; i < DEFERRED_BUFFERCOUNT; i++) {
		context->ClearRenderTargetView(renderTargets[i], color);
	}
	context->ClearDepthStencilView(zBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void Deferred::BindSecondPass(ID3D11DeviceContext * context, ID3D11RenderTargetView * backBuffer, ID3D11Buffer *cameraBuffer)
{
	context->OMSetRenderTargets(1, &backBuffer, NULL);
	float clearColor[] = { 1,1,1,1 };
	context->ClearRenderTargetView(backBuffer, clearColor);

	shaderSet->bindShadersAndLayout();
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* vertBuffer = FSQ.getVertexBuffer();
	context->IASetVertexBuffers(0, 1, &vertBuffer, &strides, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	

	context->PSGetConstantBuffers(1, 1, &cameraBuffer);

	ID3D11ShaderResourceView* srvArray[DEFERRED_BUFFERCOUNT];
	for (int i = 0; i < DEFERRED_BUFFERCOUNT; i++) {
		srvArray[i] = gBuffer[i].shaderResourceView;
	}

	context->PSSetShaderResources(0, DEFERRED_BUFFERCOUNT, srvArray);

	context->Draw(4, 0);
}
