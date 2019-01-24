#pragma once
#include "OBJLoader.h"
#include "standardClasses.h"

extern ID3D11Device* gDevice;
extern ID3D11DeviceContext* gDeviceContext;

class Object : private OBJLoader
{
private:
	float3 position = float3(0,0,0);
	float3 rotation = float3(0,0,0);
	float3 scale = float3(1,1,1);
	Array<int> meshPartSize;
	Array<Vertex> mesh;
	Array<Material> materials;
	ID3D11Buffer* vertexBuffer = nullptr;
	ID3D11Buffer* materialBuffer = nullptr;

	void createBuffers();
	void freeBuffers();
public:
	float4x4 getWorldMatrix();
	bool loadMesh(string OBJFile);
	void setPosition(float3 pos);
	void rotateX(float x);
	void rotateY(float y);
	void rotateZ(float z);
	void setScale(float3 _scale);
	void move(float3 offset);
	void draw();
	Object(string OBJFile = "");
	~Object();
};

inline void Object::createBuffers()
{
	//vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));

	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = mesh.byteSize();

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = mesh.getArrayPointer();

	gDevice->CreateBuffer(&bufferDesc, &data, &vertexBuffer);

	D3D11_BUFFER_DESC desc;
	memset(&desc, 0, sizeof(desc));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(Material);

	gDevice->CreateBuffer(&desc, nullptr, &materialBuffer);
}
inline void Object::freeBuffers()
{
	if (vertexBuffer != nullptr)vertexBuffer->Release();
	if (materialBuffer != nullptr)materialBuffer->Release();
}
inline float4x4 Object::getWorldMatrix()
{
	float4x4 mat = XMMatrixScaling(scale.x,scale.y,scale.z)*XMMatrixRotationZ(rotation.z)*XMMatrixRotationX(rotation.x)*XMMatrixRotationY(rotation.y)*XMMatrixTranslation(position.x, position.y, position.z);
	return mat;
}
/*Returns true if failed*/
inline bool Object::loadMesh(string OBJFile)
{
	if (!loadFromFile(OBJFile)) {
		createTriangularMesh(mesh, meshPartSize);
		getMeshPartMaterials(materials);
		reset();
		freeBuffers();
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

		Material m = materials[i];
		gDeviceContext->UpdateSubresource(materialBuffer,0,0,&materials[i],0,0);

		gDeviceContext->Draw(meshPartSize[i],startVert);
	}

}

inline Object::Object(string OBJFile)
{
	if(OBJFile != "")loadMesh(OBJFile);
}

inline Object::~Object()
{
	freeBuffers();
}

