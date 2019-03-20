#pragma once
#include <fstream>
#include <WICTextureLoader.h>
#include "standardClasses.h"

struct VertexRef {
	int position, uv, normal;
	VertexRef(int _position = -1, int _uv = -1, int _normal = -1) {
		position = _position;
		uv = _uv;
		normal = _normal;
	}
};
struct Material {
	float4 ambient3;
	float4 diffuse3_strength;//xyz diffuse, w strength for some strange reason
	float4 specular3_shininess;//xyz specular, w shininess
	float4 mapUsages = float4(0,0,0,0);
	void setAmbient(float3 a) {
		ambient3 = float4(a.x,a.y,a.z, ambient3.w);
	}
	void setDiffuse(float3 d) {
		diffuse3_strength = float4(d.x,d.y,d.z, diffuse3_strength.w);
	}
	void setSpecular(float3 s) {
		specular3_shininess = float4(s.x,s.y,s.z, diffuse3_strength.w);
	}
	Material(float3 _diffuse = float3(1,1,1), float3 _ambient = float3(0.2,0.2,0.2), float3 _specular = float3(0,0,0), float _shininess = 1, float _alpha = 1) {
		diffuse3_strength = float4(_diffuse.x, _diffuse.y, _diffuse.z,1);
		ambient3 = float4(_ambient.x, _ambient.y, _ambient.z, 1);
		specular3_shininess = float4(_specular.x, _specular.y, _specular.z, _shininess);
	}
};
struct MeshPart {
	string materialName = "";
	Array<Array<VertexRef>> faces;
	MeshPart(string name = "") {
		materialName = name;
	}
};
struct MaterialPart {
	string materialName;
	string ambientMap = "";
	string diffuseMap = "";
	string specularMap = "";
	Material material;
	MaterialPart(string _materialName = "noName", float3 _diffuse = float3(1, 1, 1), float3 _ambient = float3(0.2, 0.2, 0.2), float3 _specular = float3(0, 0, 0), float _shininess = 1, float _alpha = 1){
		material = Material(_diffuse, _ambient, _specular, _shininess, _alpha);
		materialName = _materialName;
	}
};

class OBJLoader
{
private:
	bool loaded = false;
	string modelName = "";
	Array<float3> vertices_position;
	Array<float2> vertices_uv;
	Array<float3> vertices_normal;

	Array<MeshPart> meshParts;

	Array<MaterialPart> materials;

	int getTotalVertexCountOfMeshPart(int index) const;
	int getTotalVertexCount() const;
	int getTotalTriangularVertexCount() const;
	Vertex createVertexFromRef(VertexRef& ref) const;
	/*loads arrays of positions, uvs, normals and faces. Returns true if failed*/
	bool loadMeshFromFile(string filename);
	bool loadMaterialFromFile(string filename);
public:
	bool hasLoaded() const;
	void reset();
	/*loads mesh and material. Dont include file format!!. Returns true if failed*/
	bool loadFromFile(string filename);
	/*return a triangular mesh and the length of each part of the mesh, divided based on material*/
	Array<Vertex> createTriangularMesh(Array<int>& partCount) const;
	void getMaterialParts(Array<MaterialPart>& _materials) const;
	void averagePointNormals();
	void averagePointTriangleNormals();

	OBJLoader(string filename = "");
	~OBJLoader();
};