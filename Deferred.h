#pragma once
#include"FullScreenQuad.h"
struct RenderTargets {
	ID3D11RenderTargetView		*renderTargetView = nullptr;
	ID3D11Texture2D				*texture = nullptr;
	ID3D11ShaderResourceView	*shaderResourceView = nullptr;

};


class Deferred
{
private:
	RenderTargets gBuffer[DEFERRED_BUFFERCOUNT];
	FullScreenQuad FSQ;
	bool CreateGBuffer(ID3D11Device* device);
	ShaderSet *shaderSet;
public:
	Deferred();
	void initDeferred(ID3D11Device* device);
	virtual ~Deferred();
	ShaderSet getShaderSet();
	void setShaderSet(ShaderSet const&);
	//Render all geometry to a texture.
	void BindFirstPass(ID3D11DeviceContext* context, ID3D11DepthStencilView* zBuffer);
	//Bind result from first pass to FSQ and draw.
	void BindSecondPass(ID3D11DeviceContext * context, ID3D11RenderTargetView * backBuffer, ID3D11Buffer *cameraBuffer);
};
