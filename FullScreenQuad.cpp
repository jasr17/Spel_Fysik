#include "FullScreenQuad.h"

FullScreenQuad::FullScreenQuad()
{
	corners[0] = Vertex(float3(-1, 1, 0), float2(0, 0), float3(0, 0, -1));
	corners[1] = Vertex(float3(1, 1, 0), float2(1, 0), float3(0, 0, -1));
	corners[2] = Vertex(float3(-1, -1, 0), float2(0, 1), float3(0, 0, -1));
	corners[3] = Vertex(float3(1, -1, 0), float2(1, 1), float3(0, 0, -1));

	
}

FullScreenQuad::~FullScreenQuad()
{
	vertexBuffer->Release();
}

void FullScreenQuad::CreateQuad(ID3D11Device * device)
{
	D3D11_BUFFER_DESC bufDesc;
	ZeroMemory(&bufDesc, sizeof(bufDesc));

	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	bufDesc.Usage = D3D11_USAGE_DEFAULT;

	bufDesc.ByteWidth = sizeof(corners);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = corners;

	device->CreateBuffer(&bufDesc, &data, &vertexBuffer);
}

ID3D11Buffer * FullScreenQuad::getVertexBuffer()const
{
	return vertexBuffer;
}


