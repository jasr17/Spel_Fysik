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

inline Object::Object()
{
}

inline Object::~Object()
{
	freeBuffers();
}

