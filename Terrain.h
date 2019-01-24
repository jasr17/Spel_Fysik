#pragma once
#include "standardClasses.h"
#include <WICTextureLoader.h>
extern ID3D11Device* gDevice;
extern ID3D11DeviceContext* gDeviceContext;
class Terrain
{
private:
	float3 position = float3(0,0,0);
	float3 rotation = float3(0,0,0);
	float3 scale = float3(1,1,1);

	XMINT2 tileCount;
	Array<Vertex> mesh;

	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11ShaderResourceView* textureView = nullptr;

	void freeGrid(Vertex** grid);
	Vertex** createGrid();
	void convertGridToMesh(Vertex** grid);
	Vertex** applyHeightToGrid(Vertex** grid, const wchar_t* filePath);
	Vertex** configureNormals(Vertex** grid);
	void createBuffers();
	void freeBuffers();
	void reset();
public:
	void rotateY(float y);
	float4x4 getWorldMatrix();
	void draw();
	bool create(XMINT2 _tileCount, float _tileSize, float height, const wchar_t* filePath);
	Terrain(XMINT2 _tileCount, float _tileSize, float height, const wchar_t* filePath);
	Terrain();
	~Terrain();
};

inline void Terrain::freeGrid(Vertex ** grid)
{
	for (int i = 0; i < tileCount.x+1; i++)
	{
		delete[] grid[i];
	}
	delete[] grid;
	grid = nullptr;
}
inline Vertex** Terrain::createGrid()
{
	//create terrain grid
	float tileLength = 1.0f / (tileCount.x > tileCount.y ? tileCount.x : tileCount.y);
	Vertex** grid = new Vertex*[tileCount.x + 1];
	for (int xx = 0; xx < tileCount.x + 1; xx++)
	{
		grid[xx] = new Vertex[tileCount.y + 1];
		for (int yy = 0; yy < tileCount.y + 1; yy++)
		{
			grid[xx][yy].position = float3(xx*tileLength - (tileLength*tileCount.x)/2, 0, yy*tileLength - (tileLength*tileCount.y)/2);
			grid[xx][yy].uv = float2((float)xx/tileCount.x, (float)yy/tileCount.y);
			grid[xx][yy].normal = float3(0,1,0);
		}
	}
	return grid;
}
inline void Terrain::convertGridToMesh(Vertex ** grid)
{
	mesh.reset();
	mesh.resize(tileCount.x*tileCount.y*6);
	int index = 0;
	for (int yy = 0; yy < tileCount.y; yy++)
	{
		for (int xx = 0; xx < tileCount.x; xx++)
		{
			//tri1
			mesh.set(index++, grid[xx + 1][yy + 1]);
			mesh.set(index++, grid[xx + 0][yy + 0]);
			mesh.set(index++, grid[xx + 0][yy + 1]);
			float3 n1 = (grid[xx + 0][yy + 0].position - grid[xx + 1][yy + 1].position).Cross(grid[xx + 0][yy + 1].position- grid[xx + 1][yy + 1].position);
			//tri2
			mesh.set(index++, grid[xx + 0][yy + 0]);
			mesh.set(index++, grid[xx + 1][yy + 1]);
			mesh.set(index++, grid[xx + 1][yy + 0]);
			float3 n2 = (grid[xx + 1][yy + 1].position - grid[xx + 0][yy + 0].position).Cross(grid[xx + 1][yy + 0].position - grid[xx + 0][yy + 0].position);

			grid[xx + 1][yy + 1].normal += n1;
			grid[xx + 0][yy + 0].normal += n1;
			grid[xx + 0][yy + 1].normal += n1;

			grid[xx + 0][yy + 0].normal += n2;
			grid[xx + 1][yy + 1].normal += n2;
			grid[xx + 1][yy + 0].normal += n2;
		}
	}
	freeGrid(grid);
}
inline Vertex ** Terrain::applyHeightToGrid(Vertex ** grid, const wchar_t* filePath)
{
	//get texture
	CoInitialize(nullptr);
	ID3D11Resource* res = nullptr;
	HRESULT hr = CreateWICTextureFromFileEx(gDevice, filePath,0,D3D11_USAGE_STAGING,0, D3D11_CPU_ACCESS_READ,0,WIC_LOADER_DEFAULT, &res, nullptr);

	ID3D11Texture2D* tex = nullptr;
	res->QueryInterface(&tex);

	D3D11_TEXTURE2D_DESC texD;
	tex->GetDesc(&texD);

	D3D11_MAPPED_SUBRESOURCE sub;
	hr = gDeviceContext->Map(res,0,D3D11_MAP_READ,0,&sub);

	CoUninitialize();
	unsigned char* data = (unsigned char*)sub.pData;
	int texWidth = texD.Width;
	int texHeight = texD.Height;
	for (int yy = 0; yy < tileCount.y+1; yy++)
	{
		for (int xx = 0; xx < tileCount.x+1; xx++)
		{
			float2 uv = grid[xx][yy].uv;
			int ix = uv.x*(texWidth-1), iy = uv.y*(texHeight-1);
			unsigned char d = 0;
			if(texD.Format == DXGI_FORMAT_R8_UNORM)
				d = data[iy * sub.RowPitch + ix];
			if (texD.Format == DXGI_FORMAT_R8G8B8A8_UNORM)
				d = data[iy * sub.RowPitch + ix*4];
			grid[xx][yy].position.y = (float)d / 255;
		}
	}

	return grid;
}
inline Vertex ** Terrain::configureNormals(Vertex ** grid)
{
	//int** gridCount = new int*[tileCount.x+1];
	//for (int xx = 0; xx < tileCount.x+1; xx++)
	//{
	//	gridCount[xx] = new int[tileCount.y+1];
	//	for (int yy = 0; yy < tileCount.y+1; yy++)
	//	{
	//		gridCount[xx][yy] = 0;
	//	}
	//}
	//int index = 0;
	//for (int yy = 0; yy < tileCount.y; yy++)
	//{
	//	for (int xx = 0; xx < tileCount.x; xx++)
	//	{
	//		//tri1
	//		float3 n1 = (grid[xx + 0][yy + 0].position - grid[xx + 1][yy + 1].position).Cross(grid[xx + 0][yy + 1].position - grid[xx + 1][yy + 1].position);
	//		grid[xx + 1][yy + 1].normal += n1; gridCount[xx + 1][yy + 1]++;
	//		grid[xx + 0][yy + 0].normal += n1; gridCount[xx + 0][yy + 0]++;
	//		grid[xx + 0][yy + 1].normal += n1; gridCount[xx + 0][yy + 1]++;
	//		//tri2
	//		float3 n2 = (grid[xx + 1][yy + 1].position - grid[xx + 0][yy + 0].position).Cross(grid[xx + 1][yy + 0].position - grid[xx + 0][yy + 0].position);
	//		grid[xx + 0][yy + 0].normal += n2; gridCount[xx + 0][yy + 0]++;
	//		grid[xx + 1][yy + 1].normal += n2; gridCount[xx + 1][yy + 1]++;
	//		grid[xx + 1][yy + 0].normal += n2; gridCount[xx + 1][yy + 0]++;
	//	}
	//}
	//for (int yy = 0; yy < tileCount.y; yy++)
	//{
	//	for (int xx = 0; xx < tileCount.x; xx++)
	//	{
	//		grid[xx + 1][yy + 1].normal /= gridCount[xx + 1][yy + 1];
	//		grid[xx + 0][yy + 0].normal /= gridCount[xx + 0][yy + 0];
	//		grid[xx + 0][yy + 1].normal /= gridCount[xx + 0][yy + 1];

	//		grid[xx + 0][yy + 0].normal /= gridCount[xx + 0][yy + 0];
	//		grid[xx + 1][yy + 1].normal /= gridCount[xx + 1][yy + 1];
	//		grid[xx + 1][yy + 0].normal /= gridCount[xx + 1][yy + 0];
	//	}
	//}
	//for (int i = 0; i < tileCount.y+1; i++)
	//{
	//	delete[] gridCount[i];
	//}
	//delete[] gridCount;

	for (int yy = 1; yy < tileCount.y; yy++)
	{
		for (int xx = 1; xx < tileCount.x; xx++)
		{

		}
	}

	return grid;
}
inline void Terrain::createBuffers()
{
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));

	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = mesh.byteSize();

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = mesh.getArrayPointer();

	gDevice->CreateBuffer(&desc, &data, &vertexBuffer);
}
inline void Terrain::freeBuffers()
{
	if (vertexBuffer != nullptr) {
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}
	if (textureView != nullptr) {
		textureView->Release();
		textureView = nullptr;
	}
}
inline void Terrain::reset()
{
	freeBuffers();
	mesh.reset();
	tileCount = XMINT2(0,0);
	position = float3(0, 0, 0);
	rotation = float3(0, 0, 0);
	scale = float3(1, 1, 1);
}
inline void Terrain::rotateY(float y)
{
	rotation.y += y;
}
inline float4x4 Terrain::getWorldMatrix()
{
	float4x4 mat = XMMatrixScaling(scale.x, scale.y, scale.z)*XMMatrixRotationZ(rotation.z)*XMMatrixRotationX(rotation.x)*XMMatrixRotationY(rotation.y)*XMMatrixTranslation(position.x, position.y, position.z);
	return mat;
}
inline void Terrain::draw()
{
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset);
	gDeviceContext->GSSetShaderResources(0, 1, &textureView);

	gDeviceContext->Draw(mesh.length(),0);
}
/*creates mesh and loads texture. Returns true if failed*/
inline bool Terrain::create(XMINT2 _tileCount, float _tileSize, float height, const wchar_t* filePath)
{
	bool check = true;
	if (_tileCount.x > 0 && _tileCount.y > 0) {
		//get texture
		CoInitialize(nullptr);
		HRESULT hr = CreateWICTextureFromFile(gDevice, gDeviceContext, filePath, nullptr, &textureView);
		if (hr != S_OK) check = false;
		CoUninitialize();
		//create grid and mesh
		tileCount = _tileCount;
		scale = float3(_tileSize,height,_tileSize);
		Vertex** g = createGrid();
		g = applyHeightToGrid(g,filePath);
		g = configureNormals(g);
		convertGridToMesh(g);
		createBuffers();
	}
	return check;
}

inline Terrain::Terrain(XMINT2 _tileCount, float _tileSize, float height, const wchar_t* filePath)
{
	create(_tileCount, _tileSize, height, filePath);
}

inline Terrain::Terrain()
{
}

inline Terrain::~Terrain()
{
	freeBuffers();
}

