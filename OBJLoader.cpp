#include "OBJLoader.h"

int OBJLoader::getVertexCountOfMeshPart(int index) const
{
	return meshParts[index].getVertexCount(); 
}

int OBJLoader::getVertexCount() const {
	int count = 0;
	for (int i = 0; i < meshParts.length(); i++)
	{
		count += meshParts[i].getVertexCount();
	}
	return count;
}

int OBJLoader::getTriangularVertexCount() const
{
	int count = 0;
	for (int i = 0; i < meshParts.length(); i++)
	{
		count += meshParts[i].getTriangularVertexCount();
	}
	return count;
}

Vertex OBJLoader::createVertexFromRef(VertexRef & ref) const
{
	Vertex v;
	if (ref.position >= 0) v.position = vertices_position[ref.position];
	if (ref.uv >= 0) v.uv = vertices_uv[ref.uv];
	if (ref.normal >= 0) v.normal = vertices_normal[ref.normal];
	return v;
}

int OBJLoader::getMaterialIndex(string materialName) const
{
	for (int i = 0; i < materials.length(); i++)
	{
		if (materials[i].materialName == materialName)
			return i;
	}
	return 0;// will produce errors if return -1
}

bool OBJLoader::hasLoaded() const
{
	return loaded;
}

void OBJLoader::reset()
{
	loaded = false;
	meshParts.reset();
	materialFileNames.reset();
	materials.reset();

	vertices_position.reset();
	vertices_normal.reset();
	vertices_uv.reset();
}

bool OBJLoader::loadMeshFromFile(string filename)
{
	fstream objFile;
	objFile.open(filename, ios::in);
	if (objFile.is_open()) {
		reset();

		string startWord = "";
		while (objFile.peek() != EOF)
		{
			objFile >> startWord;
			if (startWord[0] == '#') {//comment
				char c[100];
				objFile.getline(c, 100);
			}
			else if (startWord == "mtllib") {//mtl file name
				string name;
				objFile >> name;
				materialFileNames.add(name);
			}
			else if (startWord == "o") {//new part with part name
				string name = "";
				objFile >> name;
				meshParts.add(MeshPart(name));
			}
			else if (startWord == "usemtl") {//new faces repository for current meshPart
				string name = "";
				objFile >> name;
				meshParts.getLast().facesWithMaterial.add(FacesMaterialUse(name));
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
			else if (startWord == "f") {//face
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
				Array<Array<VertexRef>>* ar = &meshParts.getLast().facesWithMaterial.getLast().faces;
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
	mtlFile.open("Meshes/" + filename);
	if (mtlFile.is_open()) {
		//temp variables
		string text = "";
		float x = 0, y = 0, z = 0;
		float floatValue = 0;
		//read file
		string startWord = "";
		while (mtlFile.peek() != EOF)
		{
			mtlFile >> startWord;

			if (startWord == "newmtl") {//create new material
				mtlFile >> text;
				materials.add(MaterialPart(text));
			}
			else if (startWord == "Ka") {//ambient
				mtlFile >> x >> y >> z;
				materials.getLast().setAmbient(float3(x, y, z));
			}
			else if (startWord == "Kd") {//diffuse
				mtlFile >> x >> y >> z;
				materials.getLast().setDiffuse(float3(x, y, z));
			}
			else if (startWord == "Ks") {//specular
				mtlFile >> x >> y >> z;
				materials.getLast().setSpecular(float3(x, y, z));
			}
			else if (startWord == "Ns") {//shininess
				mtlFile >> floatValue;
				materials.getLast().setSpecularHighlight(floatValue);
			}
			else if (startWord == "Ni") {//diffuse strength
				mtlFile >> floatValue;
				materials.getLast().setDiffuseStrength(floatValue);
			}
			else if (startWord == "map_Ka") {
				mtlFile >> text;
				materials.getLast().setAmbientMap(text);
			}
			else if (startWord == "map_Kd") {
				mtlFile >> text;
				materials.getLast().setDiffuseMap(text);
			}
			else if (startWord == "map_Ks") {
				mtlFile >> text;
				materials.getLast().setSpecularMap(text);
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
		materials.reset();
		bool check = false;
		for (int i = 0; i < materialFileNames.length(); i++)
		{
			if (loadMaterialFromFile(materialFileNames[i]))
				check = true;
		}
		return check;
	}
	return true;
}

Array<Vertex> OBJLoader::createTriangularMesh(Array<PartInfo>& partCount) const
{
	Array<Vertex> arr;
	arr.appendCapacity(getTriangularVertexCount());
	partCount.reset();
	partCount.resize(meshParts.length());
	for (int i = 0; i < meshParts.length(); i++)//all parts
	{
		for (int k = 0; k < meshParts[i].facesWithMaterial.length(); k++)//all faces for material
		{
			int facesMaterialCount = 0;
			for (int j = 0; j < meshParts[i].facesWithMaterial[k].faces.length(); j++)//all faces
			{
				int faceLength = meshParts[i].facesWithMaterial[k].faces[j].length();
				Array<VertexRef> vr = meshParts[i].facesWithMaterial[k].faces[j];
				if (faceLength == 3) {
					arr.add(createVertexFromRef(vr[0]));
					arr.add(createVertexFromRef(vr[1]));
					arr.add(createVertexFromRef(vr[2]));
					facesMaterialCount += 3;
				}
				else if (faceLength == 4) {
					arr.add(createVertexFromRef(vr[0]));
					arr.add(createVertexFromRef(vr[1]));
					arr.add(createVertexFromRef(vr[2]));

					arr.add(createVertexFromRef(vr[2]));
					arr.add(createVertexFromRef(vr[3]));
					arr.add(createVertexFromRef(vr[0]));
					facesMaterialCount += 6;
				}
			}
			int materialIndex = getMaterialIndex(meshParts[i].facesWithMaterial[k].materialName);
			partCount[i].add(facesMaterialCount,materialIndex);
		}
	}
	return arr;
}

void OBJLoader::getMaterialParts(Array<MaterialPart>& _materials) const
{
	_materials = materials;
}
/*Each vertex gets its own normal thats averaged by all normals connected to the vertex position*/
void OBJLoader::averagePointNormals()
{
	Array<Array<int>> pointsNormalRef(vertices_position.length());
	for (int v = 0; v < vertices_position.length(); v++)
		pointsNormalRef[v].appendCapacity(5);
	//get all normals to each point
	for (int i = 0; i < meshParts.length(); i++)
	{
		for (int j = 0; j < meshParts[i].facesWithMaterial.length(); j++)
		{
			for (int f = 0; f < meshParts[i].facesWithMaterial[j].faces.length(); f++)
			{
				for (int p = 0; p < meshParts[i].facesWithMaterial[j].faces[f].length(); p++)
				{
					int point = meshParts[i].facesWithMaterial[j].faces[f][p].position;
					int normal = meshParts[i].facesWithMaterial[j].faces[f][p].normal;
					pointsNormalRef[point].add(normal);
				}
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
	for (int i = 0; i < meshParts.length(); i++)
	{
		for (int j = 0; j < meshParts[i].facesWithMaterial.length(); j++)
		{
			for (int f = 0; f < meshParts[i].facesWithMaterial[j].faces.length(); f++)
			{
				for (int p = 0; p < meshParts[i].facesWithMaterial[j].faces[f].length(); p++)
				{
					meshParts[i].facesWithMaterial[j].faces[f][p].normal = meshParts[i].facesWithMaterial[j].faces[f][p].position;
				}
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
		for (int i = 0; i < meshParts[m].facesWithMaterial.length(); i++)
		{
			for (int f = 0; f < meshParts[m].facesWithMaterial[i].faces.length(); f++)
			{
				Array<VertexRef> face = meshParts[m].facesWithMaterial[i].faces[f];
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
	}
	//apply new normals(same as position, the normals correspond to each position)
	for (int i = 0; i < meshParts.length(); i++)
	{
		for (int j = 0; j < meshParts[i].facesWithMaterial.length(); j++)
		{
			for (int f = 0; f < meshParts[i].facesWithMaterial[j].faces.length(); f++)
			{
				for (int p = 0; p < meshParts[i].facesWithMaterial[j].faces[f].length(); p++)
				{
					meshParts[i].facesWithMaterial[j].faces[f][p].normal = meshParts[i].facesWithMaterial[j].faces[f][p].position;
				}
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