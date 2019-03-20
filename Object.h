#pragma once
#include "Mesh.h"

class Object : private OBJLoader
{
private:
	float3 position = float3(0,0,0);
	float3 rotation = float3(0,0,0);
	float3 scale = float3(1,1,1);
	bool ifChanged = false;

	Mesh* mesh = nullptr;

public:
	bool popIfChanged();
	void giveMesh(Mesh* _mesh);
	float4x4 getWorldMatrix();
	void setPosition(float3 pos);
	void setRotation(float3 rot);
	void rotateX(float x);
	void rotateY(float y);
	void rotateZ(float z);
	void setScale(float3 _scale);
	void move(float3 offset);
	void draw();
	float3 getBoundingBoxPos() const;
	float3 getBoundingBoxSize() const;
	float3 getRotatedBoundingBoxSize() const;
	float3 getRotation() const;
	float3 getPosition() const;
	float3 getScale() const;
	float castRayOnObject(float3 rayPos, float3 rayDir);
	Object(float3 _position = float3(0,0,0), float3 _rotation = float3(0,0,0), float3 _scale = float3(1,1,1), Mesh* _mesh = nullptr);
	~Object();
};