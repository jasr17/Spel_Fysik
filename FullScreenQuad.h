#pragma once
#include "standardClasses.h"

class FullScreenQuad
{
private:
	
	Vertex corners[4];
	ID3D11Buffer* vertexBuffer = nullptr;
	
public:
	FullScreenQuad();
	virtual ~FullScreenQuad();
	void CreateQuad(ID3D11Device* device);
	ID3D11Buffer* getVertexBuffer()const;
	
};