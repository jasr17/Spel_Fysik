#pragma once
#include"standardClasses.h"
const int SSAO_TEXTURE_SLOT = 4;
class SSAO
{
public:
	SSAO();
	virtual ~SSAO();
	void setNosieTexture();
	
private:
	
	ID3D11Texture2D* noiseTexture;
	ID3D11ShaderResourceView *noiceShader;
};

