#pragma once
#include "OBJLoader.h"
#include "standardClasses.h"

extern ID3D11Device* gDevice;
extern ID3D11DeviceContext* gDeviceContext;

class Mesh
{
private:
	OBJLoader loader;
	float2 MinMaxXPosition = float2(-1, -1);//.x is min, .y is max
	float2 MinMaxYPosition = float2(-1, -1);
	float2 MinMaxZPosition = float2(-1, -1);
	Array<int> meshPartSize;
	Array<Vertex> mesh;
	Array<MaterialPart> materials;
	//buffers
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* materialBuffer = nullptr;
	ID3D11ShaderResourceView*** maps = nullptr;
	//functions
	std::wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}
	float triangleTest(float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2);
	float obbTest(float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale);
	bool findMinMaxValues();

	void createBuffers();
	void freeBuffers();
public:
	void draw();
	float3 getBoundingBoxPos() const;
	float3 getBoundingBoxSize() const;
	bool loadMesh(string OBJFile, typeOfShading ToS);
	float castRayOnMesh(float3 rayPos, float3 rayDir);
	Mesh(string OBJFile = "", typeOfShading ToS = typeOfShading::flatShading);
	~Mesh();
};
