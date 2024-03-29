#include "Object.h"

float3 Object::getRotatedBoundingBoxSize() const
{
	Matrix rotMat = Matrix::CreateRotationZ(rotation.z)*Matrix::CreateRotationX(rotation.x)*Matrix::CreateRotationY(rotation.y);
	float3 size = getBoundingBoxSize();
	float2 MinMaxXPosition = float2(-1, -1);//.x is min, .y is max
	float2 MinMaxYPosition = float2(-1, -1);
	float2 MinMaxZPosition = float2(-1, -1);
	float3 points[8] =
	{
		float3(size.x,size.y,size.z) ,
		float3(size.x,size.y,-size.z) ,
		float3(size.x,-size.y,size.z) ,
		float3(size.x,-size.y,-size.z) ,
		float3(-size.x,size.y,size.z) ,
		float3(-size.x,size.y,-size.z) ,
		float3(-size.x,-size.y,size.z) ,
		float3(-size.x,-size.y,-size.z)
	};
	for (int i = 0; i < 8; i++)
	{
		float3 p = XMVector3Transform(points[i], rotMat);
		if (p.x > MinMaxXPosition.y || MinMaxXPosition.y == -1)MinMaxXPosition.y = p.x;
		if (p.x < MinMaxXPosition.x || MinMaxXPosition.x == -1)MinMaxXPosition.x = p.x;

		if (p.y > MinMaxYPosition.y || MinMaxYPosition.y == -1)MinMaxYPosition.y = p.y;
		if (p.y < MinMaxYPosition.x || MinMaxYPosition.x == -1)MinMaxYPosition.x = p.y;

		if (p.z > MinMaxZPosition.y || MinMaxZPosition.y == -1)MinMaxZPosition.y = p.z;
		if (p.z < MinMaxZPosition.x || MinMaxZPosition.x == -1)MinMaxZPosition.x = p.z;
	}
	return float3((MinMaxXPosition.y - MinMaxXPosition.x) / 2, (MinMaxYPosition.y - MinMaxYPosition.x) / 2, (MinMaxZPosition.y - MinMaxZPosition.x) / 2);
}
bool Object::popIfChanged()
{
	bool r = ifChanged;
	ifChanged = false;
	return r;
}
void Object::giveMesh(Mesh * _mesh)
{
	mesh = _mesh;
}
float4x4 Object::getWorldMatrix()
{
	float4x4 mat = XMMatrixScaling(scale.x, scale.y, scale.z)*XMMatrixRotationZ(rotation.z)*XMMatrixRotationX(rotation.x)*XMMatrixRotationY(rotation.y)*XMMatrixTranslation(position.x, position.y, position.z);
	return mat;
}
void Object::setPosition(float3 pos)
{
	position = pos;
	ifChanged = true;
}
void Object::setRotation(float3 rot)
{
	rotation = rot;
	ifChanged = true;
}
void Object::rotateX(float x)
{
	rotation.x += x;
	ifChanged = true;
}
void Object::rotateY(float y)
{
	rotation.y += y;
	ifChanged = true;
}
void Object::rotateZ(float z)
{
	rotation.z += z;
	ifChanged = true;
}
void Object::setScale(float3 _scale)
{
	scale = _scale;
	ifChanged = true;
}
void Object::move(float3 offset)
{
	position += offset;
	ifChanged = true;
}
void Object::draw()
{
	if (mesh != nullptr)mesh->draw();
}
float3 Object::getRotation() const
{
	return rotation;
}
float3 Object::getPosition() const
{
	return position;
}
float3 Object::getScale() const
{
	return scale;
}
/*get bounding box position in world space*/
float3 Object::getBoundingBoxPos() const
{
	return position + mesh->getBoundingBoxPos()*scale;
}
/*get bounding box scale/size in world space*/
float3 Object::getBoundingBoxSize() const
{
	return mesh->getBoundingBoxSize()*scale;
}

float Object::castRayOnObject(float3 rayPos, float3 rayDir)
{
	if (mesh != nullptr) {
		float4x4 mWorld = getWorldMatrix();
		float4x4 mInvWorld = mWorld.Invert();
		float3 lrayPos = XMVector4Transform(float4(rayPos.x, rayPos.y, rayPos.z, 1), mInvWorld);
		float3 lrayDir = XMVector4Transform(float4(rayDir.x, rayDir.y, rayDir.z, 0), mInvWorld);
		lrayDir.Normalize();

		float t = mesh->castRayOnMesh(lrayPos, lrayDir);
		if (t > 0) {
			float3 target = XMVector3Transform(lrayPos + lrayDir * t, mWorld);
			return (target.x - rayPos.x) / rayDir.x;
		}
		else return -1;
	}
	return -1;
}

Object::Object(float3 _position, float3 _rotation, float3 _scale, Mesh* _mesh)
{
	position = _position;
	rotation = _rotation;
	scale = _scale;
	giveMesh(_mesh);
}

Object::~Object()
{

}
