#include "OBJLoader.h"

int OBJLoader::getTotalVertexCountOfMeshPart(int index) const
{
	int l = 0;
	for (int j = 0; j < meshParts[index].faces.length(); j++)
	{
		l += meshParts[index].faces[j].length();
	}
	return l;
}

int OBJLoader::getTotalVertexCount() const {
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

int OBJLoader::getTotalTriangularVertexCount() const
{
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

Vertex OBJLoader::createVertexFromRef(VertexRef & ref) const
{
	Vertex v;
	if (ref.position >= 0) v.position = vertices_position[ref.position];
	if (ref.uv >= 0) v.uv = vertices_uv[ref.uv];
	if (ref.normal >= 0) v.normal = vertices_normal[ref.normal];
	return v;
}

bool OBJLoader::hasLoaded() const
{
	return loaded;
}

void OBJLoader::reset()
{
	modelName = "";
	loaded = false;
	meshParts.reset();
}

bool OBJLoader::loadMeshFromFile(string filename)
{
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
				objFile.getline(c, 100);
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
				vertices_position.appendIfNecessary(100000);
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
				uv.y = 1 - uv.y;//UV IS UPSIDE DOWN!!!!
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
					face.add(VertexRef(pI - 1, uvI - 1, nI - 1));
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

bool OBJLoader::loadMaterialFromFile(string filename)
{
	fstream mtlFile;
	mtlFile.open(filename);
	if (mtlFile.is_open()) {
		float x = 0, y = 0, z = 0;//ambient, diffuse, specular temp variables
		float shininess = 0;//specular shininess, diffuse strength temp variable
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
				materials.getLast().material.specular3_shininess.w = shininess;
			}
			else if (startWord == "Ni") {//diffuse strength
				mtlFile >> shininess;
				materials.getLast().material.diffuse3_strength.w = shininess;
			}
			else if (startWord == "d") {//alpha
				mtlFile >> d;
				//removed cus dont need
				//materials.getLast().material.alpha = d;
			}
			else if (startWord == "map_Ka") {
				string name;
				mtlFile >> name;
				materials.getLast().ambientMap = name;
				materials.getLast().material.mapUsages.x = 1;
			}
			else if (startWord == "map_Kd") {
				string name;
				mtlFile >> name;
				materials.getLast().diffuseMap = name;
				materials.getLast().material.mapUsages.y = 1;
			}
			else if (startWord == "map_Ks") {
				string name;
				mtlFile >> name;
				materials.getLast().specularMap = name;
				materials.getLast().material.mapUsages.z = 1;
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

bool OBJLoader::loadFromFile(string filename)
{
	if (!loadMeshFromFile(filename + ".obj"))
	{
		loaded = true;
		return loadMaterialFromFile(filename + ".mtl");
	}
	return true;
}

Array<Vertex> OBJLoader::createTriangularMesh(Array<int>& partCount) const
{
	Array<Vertex> arr;
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
	return arr;
}

void OBJLoader::getMaterialParts(Array<MaterialPart>& _materials) const
{
	_materials.reset();
	_materials.appendCapacity(meshParts.length());
	for (int i = 0; i < meshParts.length(); i++)
	{
		for (int j = 0; j < materials.length(); j++)
		{
			string n1 = meshParts[i].materialName;
			string n2 = materials[j].materialName;
			if (meshParts[i].materialName == materials[j].materialName) {
				_materials.add(materials[j]);
				break;
			}
		}
	}
}
/*Each vertex gets its own normal thats averaged by all normals connected to the vertex position*/
void OBJLoader::averagePointNormals()
{
	Array<Array<int>> pointsNormalRef(vertices_position.length());
	for (int v = 0; v < vertices_position.length(); v++)
		pointsNormalRef[v].appendCapacity(5);
	//get all normals to each point
	for (int m = 0; m < meshParts.length(); m++)
	{
		for (int f = 0; f < meshParts[m].faces.length(); f++)
		{
			for (int p = 0; p < meshParts[m].faces[f].length(); p++)
			{
				int point = meshParts[m].faces[f][p].position;
				int normal = meshParts[m].faces[f][p].normal;
				pointsNormalRef[point].add(normal);
			}
		}
	}
	//take avarage
	Array<float3> pointsNormal(vertices_position.length());
	for (int i = 0; i < pointsNormalRef.length(); i++)
	{
		float3 normal(0, 0, 0);
		for (int j = 0; j < pointsNormalRef[i].length(); j++)
		{
			normal += vertices_normal[pointsNormalRef[i][j]];
		}
		normal /= pointsNormalRef[i].length();
		pointsNormal[i] = normal;
	}
	//apply new normals(same as position, the normals correspond to each position)
	for (int m = 0; m < meshParts.length(); m++)
	{
		for (int f = 0; f < meshParts[m].faces.length(); f++)
		{
			for (int p = 0; p < meshParts[m].faces[f].length(); p++)
			{
				meshParts[m].faces[f][p].normal = meshParts[m].faces[f][p].position;
			}
		}
	}
	//overwrite normals
	vertices_normal = pointsNormal;
}
/*Each vertex gets its own normal thats averaged by all the triangle normals its connected to*/
void OBJLoader::averagePointTriangleNormals()
{
	Array<float3> pointsNormals(vertices_position.length());
	//get all normals to each point
	for (int m = 0; m < meshParts.length(); m++)
	{
		for (int f = 0; f < meshParts[m].faces.length(); f++)
		{
			Array<VertexRef> face = meshParts[m].faces[f];
			if (face.length() == 3) {
				//get normal
				float3 v1 = vertices_position[face[1].position] - vertices_position[face[0].position];
				float3 v2 = vertices_position[face[2].position] - vertices_position[face[0].position];
				float3 normal = v1.Cross(v2);
				normal.Normalize();
				//add normal
				pointsNormals[face[0].position] += normal;
				pointsNormals[face[1].position] += normal;
				pointsNormals[face[2].position] += normal;
			}
			else if (face.length() == 4) {
				float3 p0 = vertices_position[face[0].position];
				float3 p1 = vertices_position[face[1].position];
				float3 p2 = vertices_position[face[2].position];
				float3 p3 = vertices_position[face[3].position];

				//get normal 1
				float3 v1 = p1 - p0;
				float3 v2 = p2 - p0;
				float3 normal = v1.Cross(v2);
				normal.Normalize();
				//add normal
				pointsNormals[face[0].position] += normal;
				pointsNormals[face[1].position] += normal;
				pointsNormals[face[2].position] += normal;

				//get normal 2
				v1 = p3 - p2;
				v2 = p0 - p2;
				normal = v1.Cross(v2);
				normal.Normalize();
				//add normal
				pointsNormals[face[2].position] += normal;
				pointsNormals[face[3].position] += normal;
				pointsNormals[face[0].position] += normal;
			}
		}
	}
	//apply new normals(same as position, the normals correspond to each position)
	for (int m = 0; m < meshParts.length(); m++)
	{
		for (int f = 0; f < meshParts[m].faces.length(); f++)
		{
			for (int p = 0; p < meshParts[m].faces[f].length(); p++)
			{
				meshParts[m].faces[f][p].normal = meshParts[m].faces[f][p].position;
			}
		}
	}
	//normalize all normals and apply to new normals
	for (int i = 0; i < pointsNormals.length(); i++)
	{
		pointsNormals[i].Normalize();
	}
	vertices_normal = pointsNormals;
}

OBJLoader::OBJLoader(string filename)
{
	if (filename != "")loadFromFile(filename);
}

OBJLoader::~OBJLoader()
{
}