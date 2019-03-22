#include "Mesh.h"

ID3D11Buffer* Mesh::materialBuffer = nullptr;
int* Mesh::meshCount = new int(0);

float Mesh::triangleTest(float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2) {
	float3 normal = ((tri1 - tri0).Cross(tri2 - tri0)); normal.Normalize();
	float3 toTri = rayOrigin - tri0; toTri.Normalize();
	float proj = toTri.Dot(normal);
	if (proj < 0)return -1;
	//mat3 m = mat3(-rayDir, tri.vtx1.xyz - tri.vtx0.xyz, tri.vtx2.xyz - tri.vtx0.xyz);
	float4x4 m(-rayDir, tri1 - tri0, tri2 - tri0);
	float3 op0 = rayOrigin - tri0;
	float3 tuv = XMVector3Transform(op0, m.Invert());
	float4 tuvw = float4(tuv.x, tuv.y, tuv.z, 0);
	tuvw.w = 1 - tuvw.y - tuvw.z;
	if (tuvw.y > 0 && tuvw.y < 1 && tuvw.z > 0 && tuvw.z < 1 && tuvw.w > 0 && tuvw.w < 1)
		return tuvw.x;
	else return -1;
}
float Mesh::obbTest(float3 rayDir, float3 rayOrigin, float3 boxPos, float3 boxScale)
{
	//SLABS CALULATIONS(my own)
	float4 data[3] = { float4(1,0,0,boxScale.x),float4(0,1,0,boxScale.y),float4(0,0,1,boxScale.z) };//{o.u_hu,o.v_hv,o.w_hw};
	float Tmin = -99999999, Tmax = 9999999999;
	for (int i = 0; i < 3; i++) {
		float3 tempNormal = float3(data[i].x, data[i].y, data[i].z);
		float3 center1 = boxPos + tempNormal * data[i].w;
		float3 center2 = boxPos - tempNormal * data[i].w;
		float npd = tempNormal.Dot(rayDir);
		if (npd != 0) {
			float t1 = (tempNormal.Dot(center1) - tempNormal.Dot(rayOrigin)) / npd;
			float t2 = (tempNormal.Dot(center2) - tempNormal.Dot(rayOrigin)) / npd;
			if (t1 > t2) {
				float temp = t1;
				t1 = t2; t2 = temp;
			}
			if (t1 > Tmin) {
				Tmin = t1;
			}
			if (t2 < Tmax) Tmax = t2;
		}
		else return -1;
	}
	if (Tmin < Tmax) return Tmin;
	else return -1;
}
bool Mesh::findMinMaxValues()
{
	if (mesh.length() > 0) {

		for (int i = 0; i < mesh.length(); i++)
		{
			float3 p = mesh[i].position;
			if (p.x > MinMaxXPosition.y || MinMaxXPosition.y == -1)MinMaxXPosition.y = p.x;
			if (p.x < MinMaxXPosition.x || MinMaxXPosition.x == -1)MinMaxXPosition.x = p.x;

			if (p.y > MinMaxYPosition.y || MinMaxYPosition.y == -1)MinMaxYPosition.y = p.y;
			if (p.y < MinMaxYPosition.x || MinMaxYPosition.x == -1)MinMaxYPosition.x = p.y;

			if (p.z > MinMaxZPosition.y || MinMaxZPosition.y == -1)MinMaxZPosition.y = p.z;
			if (p.z < MinMaxZPosition.x || MinMaxZPosition.x == -1)MinMaxZPosition.x = p.z;
		}

		return true;
	}
	else return false;
}
void Mesh::createBuffers()
{
	HRESULT check;

	freeBuffers();
	//vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = mesh.byteSize();

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = mesh.getArrayPointer();

	check = gDevice->CreateBuffer(&bufferDesc, &data, &vertexBuffer);

	//material buffer
	if (materialBuffer == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(Material);

		check = gDevice->CreateBuffer(&desc, nullptr, &materialBuffer);
	}
	//textureBuffer
	maps = new ID3D11ShaderResourceView**[materials.length()];
	for (size_t i = 0; i < materials.length(); i++)
	{
		maps[i] = new ID3D11ShaderResourceView*[3];
		if (materials[i].ambientMap != "") {
			string path = "Meshes/" + materials[i].ambientMap;
			wstring wstr = s2ws(path);
			LPCWCHAR str = wstr.c_str();
			HRESULT hr_a = CreateWICTextureFromFile(gDevice, gDeviceContext, str, nullptr, &maps[i][0]);
		}
		else maps[i][0] = nullptr;
		if (materials[i].diffuseMap != "") {
			string path = "Meshes/" + materials[i].diffuseMap;
			wstring wstr = s2ws(path);
			LPCWCHAR str = wstr.c_str();
			HRESULT hr_d = CreateWICTextureFromFile(gDevice, gDeviceContext, str, nullptr, &maps[i][1]);
		}
		else maps[i][1] = nullptr;
		if (materials[i].specularMap != "") {
			string path = "Meshes/" + materials[i].specularMap;
			wstring wstr = s2ws(path);
			LPCWCHAR str = wstr.c_str();
			HRESULT hr_s = CreateWICTextureFromFile(gDevice, gDeviceContext, str, nullptr, &maps[i][2]);
		}
		else maps[i][2] = nullptr;
	}
}
void Mesh::freeBuffers()
{
	if (vertexBuffer != nullptr) {
		vertexBuffer->Release();
		vertexBuffer = nullptr;
	}
	if (materialBuffer != nullptr && meshCount == 0) {
		materialBuffer->Release();
		materialBuffer = nullptr;
	}
	if (maps != nullptr) {
		for (int i = 0; i < materials.length(); i++)
		{
			if (maps[i] != nullptr) {
				if (maps[i][0] != nullptr)maps[i][0]->Release();
				if (maps[i][1] != nullptr)maps[i][1]->Release();
				if (maps[i][2] != nullptr)maps[i][2]->Release();
				delete[] maps[i];
			}
		}
		delete[] maps;
		maps = nullptr;
	}
}
void Mesh::draw()
{
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &strides, &offset);
	gDeviceContext->PSSetConstantBuffers(2, 1, &materialBuffer);

	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	int startVert = 0;
	for (int i = 0; i < meshPartInfo.length(); i++)
	{
		for (int k = 0; k < meshPartInfo[i].length(); k++)
		{
			int materialIndex = meshPartInfo[i].getMaterialIndex(k);
			gDeviceContext->UpdateSubresource(materialBuffer, 0, 0, &materials[materialIndex].material, 0, 0);

			gDeviceContext->PSSetShaderResources(0, 3, maps[materialIndex]);

			gDeviceContext->Draw(meshPartInfo[i].getVertexCount(k), startVert);

			startVert += meshPartInfo[i].getVertexCount(k);
		}
	}
}
float3 Mesh::getBoundingBoxPos() const
{
	return float3((MinMaxXPosition.x + MinMaxXPosition.y) / 2, (MinMaxYPosition.x + MinMaxYPosition.y) / 2, (MinMaxZPosition.x + MinMaxZPosition.y) / 2);
}
float3 Mesh::getBoundingBoxSize() const
{
	return float3((MinMaxXPosition.y - MinMaxXPosition.x) / 2, (MinMaxYPosition.y - MinMaxYPosition.x) / 2, (MinMaxZPosition.y - MinMaxZPosition.x) / 2);
}
bool Mesh::loadMesh(string OBJFile, typeOfShading ToS)
{
	if (loader.loadFromFile(OBJFile)) {
		if (ToS == typeOfShading::smoothShading)loader.averagePointTriangleNormals();
		mesh = loader.createTriangularMesh(meshPartInfo);
		findMinMaxValues();
		loader.getMaterialParts(materials);
		loader.reset();
		createBuffers();
		return true;
	}
	else {
		return false;
		//Error loading file
	}
}
float Mesh::castRayOnMesh(float3 rayPos, float3 rayDir)
{
	//get bounding box in local space
	float3 bPos = getBoundingBoxPos();
	float3 bScale = getBoundingBoxSize();
	//check if close
	if (obbTest(rayDir, rayPos, bPos, bScale) != -1) {
		//find the exact point
		float closest = -1;
		int length = mesh.length() / 3;
		for (int i = 0; i < length; i++)
		{
			int index = i * 3;
			float3 v0 = mesh[index + 0].position;
			float3 v1 = mesh[index + 1].position;
			float3 v2 = mesh[index + 2].position;
			float t = triangleTest(rayDir, rayPos, v0, v1, v2);
			if ((t > 0 && t < closest) || closest < 0)closest = t;
		}
		return closest;
	}
	return -1;
}
Mesh::Mesh(string OBJFile, typeOfShading ToS)
{
	meshCount++;
	if (OBJFile != "")loadMesh(OBJFile, ToS);
}
Mesh::~Mesh()
{
	meshCount--;
	freeBuffers();
}

