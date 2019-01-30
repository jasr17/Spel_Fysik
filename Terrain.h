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
	Vertex** grid = nullptr;
	Array<Vertex> mesh;

	ID3D11Buffer* vertexBuffer = nullptr;

	void freeGrid(Vertex** grid);
	void createGrid();
	void convertGridToMesh_flatShading();
	void convertGridToMesh_smoothShading();
	void applyHeightToGrid(const wchar_t* filePath);
	void smoothNormals();
	void smoothSurface(float s);
	void createBuffers();
	void freeBuffers();
	void reset();
public:
	void rotateY(float y);
	float4x4 getWorldMatrix();
	float getHeightOfTerrainFromCoordinates(float x, float y);
	void draw();
	bool create(XMINT2 _tileCount, float desiredMapSize, float height, const wchar_t* filePath);
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
inline void Terrain::createGrid()
{
	//create terrain grid
	float2 tileLength = float2(1.0f / tileCount.x, 1.0f/tileCount.y);
	grid = new Vertex*[tileCount.x + 1];
	for (int xx = 0; xx < tileCount.x + 1; xx++)
	{
		grid[xx] = new Vertex[tileCount.y + 1];
		for (int yy = 0; yy < tileCount.y + 1; yy++)
		{
			grid[xx][yy].position = float3(xx*tileLength.x - (tileLength.x*tileCount.x)/2, 0, yy*tileLength.y - (tileLength.y*tileCount.y)/2);
			grid[xx][yy].uv = float2((float)xx/tileCount.x, (float)yy/tileCount.y);
			grid[xx][yy].normal = float3(0,1,0);
		}
	}
}
inline void Terrain::convertGridToMesh_flatShading()
{
	mesh.reset();
	mesh.appendCapacity(tileCount.x*tileCount.y*6);
	for (int yy = 0; yy < tileCount.y; yy++)
	{
		for (int xx = 0; xx < tileCount.x; xx++)
		{
			//add the position and uv but calculate the normal for this specific triangle to get the flat shading effect.
			float3 n1 = (grid[xx + 0][yy + 0].position - grid[xx + 1][yy + 1].position).Cross(grid[xx + 0][yy + 1].position - grid[xx + 1][yy + 1].position);
			float3 n2 = (grid[xx + 1][yy + 1].position - grid[xx + 0][yy + 0].position).Cross(grid[xx + 1][yy + 0].position - grid[xx + 0][yy + 0].position);
			//tri 1
			mesh.add(Vertex(grid[xx + 1][yy + 1].position, grid[xx + 1][yy + 1].uv, n1));
			mesh.add(Vertex(grid[xx + 0][yy + 0].position, grid[xx + 0][yy + 0].uv, n1));
			mesh.add(Vertex(grid[xx + 0][yy + 1].position, grid[xx + 0][yy + 1].uv, n1));
			//tri 2
			mesh.add(Vertex(grid[xx + 0][yy + 0].position, grid[xx + 0][yy + 0].uv, n2));
			mesh.add(Vertex(grid[xx + 1][yy + 1].position, grid[xx + 1][yy + 1].uv, n2));
			mesh.add(Vertex(grid[xx + 1][yy + 0].position, grid[xx + 1][yy + 0].uv, n2));
		}
	}
}
inline void Terrain::convertGridToMesh_smoothShading()
{
	smoothNormals();
	mesh.reset();
	mesh.appendCapacity(tileCount.x*tileCount.y * 6);
	for (int yy = 0; yy < tileCount.y; yy++)
	{
		for (int xx = 0; xx < tileCount.x; xx++)
		{
			//tri 1
			mesh.add(grid[xx + 1][yy + 1]);
			mesh.add(grid[xx + 0][yy + 0]);
			mesh.add(grid[xx + 0][yy + 1]);
			//tri 2
			mesh.add(grid[xx + 0][yy + 0]);
			mesh.add(grid[xx + 1][yy + 1]);
			mesh.add(grid[xx + 1][yy + 0]);
		}
	}
}
inline void Terrain::applyHeightToGrid(const wchar_t* filePath)
{
	//get texture
	ID3D11Resource* res = nullptr;
	HRESULT hr = CreateWICTextureFromFileEx(gDevice, filePath,0,D3D11_USAGE_STAGING,0, D3D11_CPU_ACCESS_READ,0,WIC_LOADER_DEFAULT, &res, nullptr);

	ID3D11Texture2D* tex = nullptr;
	res->QueryInterface(&tex);

	D3D11_TEXTURE2D_DESC texD;
	tex->GetDesc(&texD);

	D3D11_MAPPED_SUBRESOURCE sub;
	hr = gDeviceContext->Map(res,0,D3D11_MAP_READ,0,&sub);

	int texWidth = texD.Width;
	int texHeight = texD.Height;
	for (int yy = 0; yy < tileCount.y+1; yy++)
	{
		for (int xx = 0; xx < tileCount.x+1; xx++)
		{
			float2 uv = grid[xx][yy].uv;
			int ix = uv.x*(texWidth-1), iy = uv.y*(texHeight-1);
			if (texD.Format == DXGI_FORMAT_R8_UNORM) {
				unsigned char d = ((unsigned char*)sub.pData)[iy * sub.RowPitch + ix];
				grid[xx][yy].position.y = (float)d / (pow(2, 1 * 8) - 1);
			}	
			else if (texD.Format == DXGI_FORMAT_R8G8B8A8_UNORM) {
				unsigned char d = ((unsigned char*)sub.pData)[iy * sub.RowPitch + ix*4];
				grid[xx][yy].position.y = (float)d / (pow(2, 1 * 8) - 1);
			}
			else if (texD.Format == DXGI_FORMAT_R16G16B16A16_UNORM) {
				unsigned short int d = ((unsigned short int*)sub.pData)[iy*(sub.RowPitch/sizeof(short int)) + ix*4];
				grid[xx][yy].position.y = (float)d / (pow(2, sizeof(short int)*8) - 1);
			}
			else {
				grid[xx][yy].position.y = 0;
			}
			
		}
	}
}
/*sets the average normal for all point in grid*/
inline void Terrain::smoothNormals()
{
	//reset normals
	for (int yy = 0; yy < tileCount.y; yy++)
	{
		for (int xx = 0; xx < tileCount.x; xx++)
		{
			grid[xx][yy].normal = float3(0, 0, 0);
		}
	}
	//sum all normals
	int index = 0;
	float3 normal;
	for (int yy = 0; yy < tileCount.y; yy++)
	{
		for (int xx = 0; xx < tileCount.x; xx++)
		{
			//tri1
			normal = (grid[xx + 0][yy + 0].position - grid[xx + 1][yy + 1].position).Cross(grid[xx + 0][yy + 1].position - grid[xx + 1][yy + 1].position);
			normal.Normalize();
			grid[xx + 1][yy + 1].normal += normal;
			grid[xx + 0][yy + 0].normal += normal;
			grid[xx + 0][yy + 1].normal += normal;
			//tri2
			normal = (grid[xx + 1][yy + 1].position - grid[xx + 0][yy + 0].position).Cross(grid[xx + 1][yy + 0].position - grid[xx + 0][yy + 0].position);
			normal.Normalize();
			grid[xx + 0][yy + 0].normal += normal;
			grid[xx + 1][yy + 1].normal += normal;
			grid[xx + 1][yy + 0].normal += normal;
		}
	}
	//normalize all normals
	for (int yy = 0; yy < tileCount.y+1; yy++)
	{
		for (int xx = 0; xx < tileCount.x+1; xx++)
		{
			grid[xx][yy].normal.Normalize();
		}
	}
}
inline void Terrain::smoothSurface(float s)
{
	//create temp grid
	float3** tempGrid = new float3*[tileCount.x+1];
	for (int i = 0; i < tileCount.x+1; i++)
	{
		tempGrid[i] = new float3[tileCount.y+1];
		for (int j = 0; j < tileCount.y+1; j++)
		{
			tempGrid[i][j] = grid[i][j].position;
		}
	}
	//smooth
	for (int yy = 1; yy < tileCount.y; yy++)
	{
		for (int xx = 1; xx < tileCount.x; xx++)
		{
			float3 mp = (tempGrid[xx + 1][yy + 0] + tempGrid[xx + 0][yy + 1] + tempGrid[xx + -1][yy + 0] + tempGrid[xx + 0][yy + -1]) / 4;
			grid[xx][yy].position += (mp-grid[xx][yy].position)*s;
		}
	}
	//free
	for (int i = 0; i < tileCount.x; i++)
	{
		delete[] tempGrid[i];
	}
	delete[] tempGrid;
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
inline float Terrain::getHeightOfTerrainFromCoordinates(float x, float y)
{
	float X = x / scale.x + 0.5;
	float Y = y / scale.z + 0.5;
	if (X > 0 && X < 1 && Y > 0 && Y < 1) {
		float fx = X * tileCount.x;
		float fy = Y * tileCount.y;
		int ix = fx;
		int iy = fy;
		float3 p1 = grid[ix + 0][iy + 0].position;
		float3 p2 = grid[ix + 1][iy + 0].position;
		float3 p3 = grid[ix + 0][iy + 1].position;
		float3 p4 = grid[ix + 1][iy + 1].position;
		float3 p12 = p1 + (p2 - p1)*(ix - fx);
		float3 p34 = p3 + (p4 - p3)*(ix - fx);
		float3 p1234 = p12 + (p34 - p12)*(iy - fy);
		return p1234.y*scale.y;
	}
	else {
		return 0;
	}
}
inline void Terrain::draw()
{
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset);

	gDeviceContext->Draw(mesh.length(),0);
}
/*creates mesh and loads texture. Returns true if failed*/
inline bool Terrain::create(XMINT2 _tileCount, float desiredMapSize, float height, const wchar_t* filePath)
{
	bool check = true;
	if (_tileCount.x > 0 && _tileCount.y > 0) {
		//create grid and mesh
		tileCount = _tileCount;
		if (_tileCount.x > _tileCount.y) {
			scale = float3(desiredMapSize,height, desiredMapSize*((float)_tileCount.y/_tileCount.x));
		}
		else {
			scale = float3(desiredMapSize*((float)_tileCount.x/_tileCount.y), height, desiredMapSize);
		}
		createGrid();
		applyHeightToGrid(filePath);
		smoothSurface(0.5);
		convertGridToMesh_smoothShading();
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
	freeGrid(grid);
	freeBuffers();
}

