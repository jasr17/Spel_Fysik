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
	Material(float3 _diffuse = float3(1,1,1), float3 _ambient = float3(0.2,0.2,0.2), float3 _specular = float3(0,0,0), float _shininess = 1, float _alpha = 1) {
		diffuse3_strength = float4(_diffuse.x, _diffuse.y, _diffuse.z,1);
		ambient3 = float4(_ambient.x, _ambient.y, _ambient.z, 1);
		specular3_shininess = float4(_specular.x, _specular.y, _specular.z, _shininess);
	}
};
struct FacesMaterialUse {
	string materialName = "";//the faces material usage
	Array<Array<VertexRef>> faces;//array with the face points in an array with all faces.
	int getVertexCount() {
		int count = 0;
		for (int i = 0; i < faces.length(); i++)
		{
			count += faces[i].length();
		}
		return count;
	}
	int getTriangularVertexCount() {
		int count = 0;
		int length = 0;
		for (int i = 0; i < faces.length(); i++)
		{
			length = faces[i].length();
			if (length == 3)
				count += 3;
			else if (length == 4)//4 points is 2 triangles and 2 triangles is 6 vertices.
				count += 6;
		}
		return count;
	}
	FacesMaterialUse(string mtlName = "") {
		materialName = mtlName;
	}
};
struct MeshPart {
	string partName = "";//object name
	Array<FacesMaterialUse> facesWithMaterial;
	int getVertexCount() {
		int count = 0;
		for (int i = 0; i < facesWithMaterial.length(); i++)
		{
			count += facesWithMaterial[i].getVertexCount();
		}
		return count;
	}
	int getTriangularVertexCount() {
		int count = 0;
		for (int i = 0; i < facesWithMaterial.length(); i++)
		{
			count += facesWithMaterial[i].getTriangularVertexCount();
		}
		return count;
	}
	MeshPart(string name = "") {
		partName = name;
	}
};
struct PartInfo {
private:
	Array<int> vertexCount;//amount of vertices using a material
	Array<int> materialUsage;//index of material
public:
	void add(int _vertexCount, int _materialUsage) {
		vertexCount.add(_vertexCount);
		materialUsage.add(_materialUsage);
	}
	int getVertexCount(int index) {
		return vertexCount[index];
	}
	int getMaterialIndex(int index) {
		return materialUsage[index];
	}
	int getSumOfVertexCountToIndex(int index) {
		int count = 0;
		for (int i = 0; i < index; i++)
		{
			count += vertexCount[i];
		}
		return count;
	}
	int getSumOfVertexCount() {
		int count = 0;
		for (int i = 0; i < vertexCount.length(); i++)
		{
			count += vertexCount[i];
		}
		return count;
	}
	int length() {
		return vertexCount.length();
	}
};
struct MaterialPart {
	string materialName;
	string ambientMap = "";
	string diffuseMap = "";
	string specularMap = "";
	Material material;
	void setAmbientMap(string map_Ka) {
		ambientMap = map_Ka;
		material.mapUsages.x = 1;
	}
	void setDiffuseMap(string map_Kd) {
		diffuseMap = map_Kd;
		material.mapUsages.y = 1;
	}
	void setSpecularMap(string map_Ks) {
		diffuseMap = map_Ks;
		material.mapUsages.z = 1;
	}
	//material funcs
	void setAmbient(float3 a) {
		material.ambient3 = float4(a.x, a.y, a.z, material.ambient3.w);
	}
	void setDiffuse(float3 d) {
		material.diffuse3_strength = float4(d.x, d.y, d.z, material.diffuse3_strength.w);
	}
	void setSpecular(float3 s) {
		material.specular3_shininess = float4(s.x, s.y, s.z, material.diffuse3_strength.w);
	}
	void setSpecularHighlight(float Ns) {
		material.specular3_shininess.w = Ns;
	}
	void setDiffuseStrength(float Ni) {
		material.diffuse3_strength.w = Ni;
	}
	//constructor
	MaterialPart(string _materialName = "noName", float3 _diffuse = float3(1, 1, 1), float3 _ambient = float3(0.2, 0.2, 0.2), float3 _specular = float3(0, 0, 0), float _shininess = 1, float _alpha = 1){
		material = Material(_diffuse, _ambient, _specular, _shininess, _alpha);
		materialName = _materialName;
	}
};

class OBJLoader
{
private:
	bool loaded = false;

	Array<float3> vertices_position;
	Array<float2> vertices_uv;
	Array<float3> vertices_normal;

	Array<MeshPart> meshParts;

	Array<string> materialFileNames;
	Array<MaterialPart> materials;

	int getVertexCountOfMeshPart(int index) const;
	int getVertexCount() const;
	int getTriangularVertexCount() const;
	Vertex createVertexFromRef(VertexRef& ref) const;
	int getMaterialIndex(string materialName) const;
	/*loads arrays of positions, uvs, normals and faces. Returns true if failed*/
	bool loadMeshFromFile(string filename);
	bool loadMaterialFromFile(string filename);
public:
	bool hasLoaded() const;
	void reset();
	/*loads mesh and material. Dont include file format!!. Returns true if failed*/
	bool loadFromFile(string filename);
	/*return a triangular mesh and the length of each part of the mesh, divided based on material*/
	Array<Vertex> createTriangularMesh(Array<PartInfo>& partCount) const;
	void getMaterialParts(Array<MaterialPart>& _materials) const;
	void averagePointNormals();
	void averagePointTriangleNormals();

	OBJLoader(string filename = "");
	~OBJLoader();
};