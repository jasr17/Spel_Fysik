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
	float3 getTerrainSize() const;
	void rotateY(float y);
	float4x4 getWorldMatrix();
	float getHeightOfTerrainFromCoordinates(float x, float y);
	float3 getPointOnTerrainFromCoordinates(float x, float z);
	void draw();
	bool create(XMINT2 _tileCount, float desiredMapSize, float height, const wchar_t* filePath, typeOfShading shading);
	Terrain(XMINT2 _tileCount, float _tileSize, float height, const wchar_t* filePath, typeOfShading shading);
	Terrain();
	~Terrain();
};