#pragma once
#include "OBJLoader.h"
#include "standardClasses.h"

extern ID3D11Device* gDevice;
extern ID3D11DeviceContext* gDeviceContext;
enum typeOfShading {
	flatShading,
	smoothShading
};
class Object : private OBJLoader
{
private:
	float3 position = float3(0,0,0);
	float3 rotation = float3(0,0,0);
	float3 scale = float3(1,1,1);
	Array<int> meshPartSize;
	Array<Vertex> mesh;
	Array<MaterialPart> materials;
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* materialBuffer = nullptr;
	ID3D11ShaderResourceView*** maps;

	void createBuffers();
	void freeBuffers();

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
	float sphereTest(float3 rayDir, float3 rayOrigin, float3 centre, float radius);
public:
	Array<Vertex>& getMesh();
	float4x4 getWorldMatrix();
	bool loadMesh(string OBJFile, typeOfShading ToS);
	void setPosition(float3 pos);
	void rotateX(float x);
	void rotateY(float y);
	void rotateZ(float z);
	void setScale(float3 _scale);
	void move(float3 offset);
	void draw();
	float castRayOnMesh(float3 rayPos, float3 rayDir);
	Object();
	~Object();
};

inline void Object::createBuffers()
{
	freeBuffers();
	//vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = mesh.byteSize();

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = mesh.getArrayPointer();

	gDevice->CreateBuffer(&bufferDesc, &data, &vertexBuffer);
	//material buffer
	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(Material);

	gDevice->CreateBuffer(&desc, nullptr, &materialBuffer);
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
			string path = "Meshes/"+materials[i].diffuseMap;
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
inline void Object::freeBuffers()
{
	if (vertexBuffer != nullptr)vertexBuffer->Release();
	if (materialBuffer != nullptr)materialBuffer->Release();
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
inline float Object::triangleTest(float3 rayDir, float3 rayOrigin, float3 tri0, float3 tri1, float3 tri2)
{
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
inline float Object::sphereTest(float3 rayDir, float3 rayOrigin, float3 centre, float radius)
{
	float3 oc = rayOrigin - centre;
	if (oc.Length() < radius)return -1;//if inside cirkel
	float a = rayDir.Dot(oc);
	float b = oc.Dot(oc) - pow(radius, 2);
	float f = pow(a, 2) - b;
	float sqrtF = sqrt(f);
	float t1 = -a + sqrtF;
	float t2 = -a - sqrtF;
	if (f < 0) { //if imaginary number
		return -1;
	}
	return min(t1, t2);
}
inline Array<Vertex>& Object::getMesh()
{
	return mesh;
}
inline float4x4 Object::getWorldMatrix()
{
	float4x4 mat = XMMatrixScaling(scale.x,scale.y,scale.z)*XMMatrixRotationZ(rotation.z)*XMMatrixRotationX(rotation.x)*XMMatrixRotationY(rotation.y)*XMMatrixTranslation(position.x, position.y, position.z);
	return mat;
}
/*Returns true if failed*/
inline bool Object::loadMesh(string OBJFile, typeOfShading ToS)
{
	if (!loadFromFile(OBJFile)) {
		if(ToS == typeOfShading::smoothShading)averagePointTriangleNormals();
		mesh = createTriangularMesh(meshPartSize);
		getMaterialParts(materials);
		reset();
		createBuffers();
		return false;
	}
	else {
		return true;
		//Error loading file
	}
}
inline void Object::setPosition(float3 pos)
{
	position = pos;
}
inline void Object::rotateX(float x)
{
	rotation.x += x;
}
inline void Object::rotateY(float y)
{
	rotation.y += y;
}
inline void Object::rotateZ(float z)
{
	rotation.z += z;
}
inline void Object::setScale(float3 _scale)
{
	scale = _scale;
}
inline void Object::move(float3 offset)
{
	position += offset;
}
inline void Object::draw()
{
	UINT strides = sizeof(Vertex);
	UINT offset = 0;
	gDeviceContext->IASetVertexBuffers(0,1,&vertexBuffer,&strides,&offset);
	gDeviceContext->PSSetConstantBuffers(2,1,&materialBuffer);

	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int i = 0; i < meshPartSize.length(); i++)
	{
		int startVert = 0;
		for (int j = 0; j < i; j++)
			startVert += meshPartSize[j];
		gDeviceContext->UpdateSubresource(materialBuffer,0,0,&materials[i].material,0,0);

		gDeviceContext->PSSetShaderResources(0,3,maps[i]);

		gDeviceContext->Draw(meshPartSize[i],startVert);
	}

}
/*cast a ray in world space and return position of collision*/
inline float Object::castRayOnMesh(float3 rayPos, float3 rayDir)
{
	float4x4 mWorld = getWorldMatrix();
	float4x4 mInvWorld = mWorld.Invert();
	float3 lrayPos = XMVector4Transform(float4(rayPos.x,rayPos.y,rayPos.z,1),mInvWorld);
	float3 lrayDir = XMVector4Transform(float4(rayDir.x,rayDir.y,rayDir.z,0),mInvWorld);
	lrayDir.Normalize();
	//check if close
	//if (sphereTest(rayDir, rayPos, float3(0, 0, 0), 1) > 0) {
		//find the exact point
		float closest = -1;
		int length = mesh.length() / 3;
		for (int i = 0; i < length; i++)
		{
			int index = i * 3;
			float3 v0 = mesh[index + 0].position;
			float3 v1 = mesh[index + 1].position;
			float3 v2 = mesh[index + 2].position;
			float t = triangleTest(lrayDir, lrayPos, v0, v1, v2);
			if ((t > 0 && t < closest) || closest < 0)closest = t;
		}
		if (closest > 0) {
			float3 target = XMVector3Transform(lrayPos + lrayDir * closest, mWorld);
			return (target.x - rayPos.x) / rayDir.x;
		}
		else return -1;
	//}
	return -1;
}

inline Object::Object()
{
}

inline Object::~Object()
{
	freeBuffers();
}

