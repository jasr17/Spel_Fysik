#pragma once
#include <fstream>
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
	float4 ambient;
	float4 diffuse;
	float4 specular;//xyz specular, w shininess
	void setAmbient(float3 a) {
		ambient = float4(a.x,a.y,a.z,ambient.w);
	}
	void setDiffuse(float3 d) {
		diffuse = float4(d.x,d.y,d.z,diffuse.w);
	}
	void setSpecular(float3 s) {
		specular = float4(s.x,s.y,s.z,specular.w);
	}
	Material(float3 _diffuse = float3(1,1,1), float3 _ambient = float3(0.2,0.2,0.2), float3 _specular = float3(0,0,0), float _shininess = 1, float _alpha = 1) {
		diffuse = float4(_diffuse.x, _diffuse.y, _diffuse.z,1);
		ambient = float4(_ambient.x, _ambient.y, _ambient.z, 1);
		specular = float4(_specular.x, _specular.y, _specular.z, _shininess);
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

	int getTotalVertexCount() const {
		int l = 0;
		for (int i = 0; i < meshParts.length(); i++)
		{
			for (int j = 0; j < meshParts[i].faces.length(); j++)
			{
				l += meshParts[i].faces[j].length();
			}
		}
		return l;
	}
	int getTotalTriangularVertexCount() const {
		int l = 0;
		int t = 0;
		for (int i = 0; i < meshParts.length(); i++)
		{
			for (int j = 0; j < meshParts[i].faces.length(); j++)
			{
				t = meshParts[i].faces[j].length();
				if (t == 3)
					l += 3;
				else if (t == 4)
					l += 6;
			}
		}
		return l;
	}
	Vertex createVertexFromRef(VertexRef& ref) const {
		Vertex v;
		if (ref.position >= 0) v.position = vertices_position[ref.position];
		if (ref.uv >= 0) v.uv = vertices_uv[ref.uv];
		if (ref.normal >= 0) v.normal = vertices_normal[ref.normal];
		return v;
	}
public:
	bool hasLoaded() const {
		return loaded;
	}
	void reset() {
		modelName = "";
		loaded = false;
		meshParts.reset();
	}
	/*loads arrays of positions, uvs, normals and faces. Returns true if failed*/
	bool loadMeshFromFile(string filename) {
		fstream objFile;
		objFile.open(filename, ios::in);
		if (objFile.is_open()) {
			reset();

			int uvIndex = 0;
			int normalIndex = 0;
			string startWord = "";
			while (objFile.peek() != EOF)
			{
				objFile >> startWord;
				if (startWord[0] == '#') {//comment
					char c[100];
					objFile.getline(c,100);
				}
				else if (startWord == "mtllib") {//modelName
					string name;
					objFile >> name;
					modelName = name;
				}
				else if (startWord == "usemtl") {
					string name = "";
					objFile >> name;
					meshParts.add(MeshPart(name));
				}
				else if (startWord == "v") {//vertex position
					float3 pos;
					objFile >> pos.x >> pos.y >> pos.z;
					if (vertices_position.appendIfNecessary(100000)) {
						int k = 0;
						k = 4;
					}
					vertices_position.add(pos);
				}
				else if (startWord == "vn") {//normal
					float3 norm;
					objFile >> norm.x >> norm.y >> norm.z;
					vertices_normal.appendIfNecessary(100000);
					vertices_normal.add(norm);
				}
				else if (startWord == "vt") {//uv coord
					float2 uv;
					objFile >> uv.x >> uv.y;
					vertices_uv.appendIfNecessary(100000);
					vertices_uv.add(uv);
				}
				else if (startWord == "f") {//face,convert to triangles
					Array<VertexRef> face;
					while (objFile.peek() != '\n' && objFile.peek() != EOF)
					{
						int pI = -1, uvI = -1, nI = -1;
						char t;
						//position index
						objFile >> pI;
						if (objFile.peek() == '/') {
							objFile >> t;
							if (objFile.peek() == '/') {
								objFile >> t >> nI;
							}
							else {
								objFile >> uvI;
								if (objFile.peek() == '/') {
									objFile >> t >> nI;
								}
							}
						}
						face.add(VertexRef(pI-1,uvI-1,nI-1));
					}
					Array<Array<VertexRef>>* ar = &meshParts[meshParts.length() - 1].faces;
					ar->appendIfNecessary(50);
					ar->add(face);
				}
			}
			objFile.close();
			return false;
		}
		return true;
	}
	bool loadMaterialFromFile(string filename) {
		fstream mtlFile;
		mtlFile.open(filename);
		if (mtlFile.is_open()) {
			float x = 0, y = 0, z = 0;//ambient, diffuse, specular temp variables
			float shininess = 0;//specular shininess temp variable
			float d = 0;//alpha
			string startWord = "";
			while (mtlFile.peek() != EOF)
			{
				mtlFile >> startWord;
				if (startWord == "newmtl") {//create new material
					string name;
					mtlFile >> name;
					materials.add(MaterialPart(name));
				}
				else if (startWord == "Ka") {//ambient
					mtlFile >> x >> y >> z;
					materials.getLast().material.setAmbient(float3(x, y, z));
				}
				else if (startWord == "Kd") {//diffuse
					mtlFile >> x >> y >> z;
					materials.getLast().material.setDiffuse(float3(x, y, z));
				}
				else if (startWord == "Ks") {//specular
					mtlFile >> x >> y >> z;
					materials.getLast().material.setSpecular(float3(x, y, z));
				}
				else if (startWord == "Ns") {//shininess
					mtlFile >> shininess;
					materials.getLast().material.specular.w = shininess;
				}
				else if (startWord == "d") {//alpha
					mtlFile >> d;
					//removed cus dont need
					//materials.getLast().material.alpha = d;
				}
				else {//remove leftover from line
					char c[100];
					mtlFile.getline(c, 100);
				}
			}
			return false;
		}
		return true;
	}
	/*loads mesh and material. Dont include file format!!. Returns true if failed*/
	bool loadFromFile(string filename) {
		if (!loadMeshFromFile(filename + ".obj"))
		{
			loaded = true;
			return loadMaterialFromFile(filename + ".mtl");
		}
		return false;
	}
	/*return a triangular mesh and the length of each part of the mesh, divided based on material*/
	void createTriangularMesh(Array<Vertex>& arr, Array<int>& partCount) const {
		arr.reset();
		arr.appendCapacity(getTotalTriangularVertexCount());
		partCount.reset();
		partCount.resize(meshParts.length());
		partCount.fill(0);
		for (int i = 0; i < meshParts.length(); i++)
		{
			for (int j = 0; j < meshParts[i].faces.length(); j++)
			{
				Array<VertexRef> vr = meshParts[i].faces[j];
				if (meshParts[i].faces[j].length() == 3) {
					arr.add(createVertexFromRef(vr[0]));
					arr.add(createVertexFromRef(vr[1]));
					arr.add(createVertexFromRef(vr[2]));
					partCount[i] += 3;
				}
				else if (meshParts[i].faces[j].length() == 4) {
					arr.add(createVertexFromRef(vr[0]));
					arr.add(createVertexFromRef(vr[1]));
					arr.add(createVertexFromRef(vr[2]));

					arr.add(createVertexFromRef(vr[2]));
					arr.add(createVertexFromRef(vr[3]));
					arr.add(createVertexFromRef(vr[0]));
					partCount[i] += 6;
				}
			}
		}
	}
	void getMeshPartMaterials(Array<Material>& _materials) const {
		_materials.reset();
		_materials.resize(meshParts.length());
		for (int i = 0; i < meshParts.length(); i++)
		{
			for (int j = 0; j < materials.length(); j++)
			{
				if (meshParts[i].materialName == materials[j].materialName) {
					_materials[i] = materials[i].material;
					break;
				}
			}
		}
	}

	OBJLoader(string filename = "") {
		if (filename != "")loadFromFile(filename);
	}
	~OBJLoader() {

	}
};