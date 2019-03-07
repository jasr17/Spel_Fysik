#pragma once
#include "standardClasses.h"

enum FrustumSides{Near, Left, Right, Far, Top, Bottom};

class Frustum
{	
struct FrustumPlane {
	float3 mPoint;
	float3 mNormal;  // Normals pointing inwards
};

private:
	FrustumPlane mPlanes[6];

public:

	Frustum();
	~Frustum();
	 const FrustumPlane* getPlanes() ;
	void constructFrustum(float3 camPos, float3 camDir, float3 up, float fowAngle, float aspectRatio, float nearZ, float farZ);
};


Frustum::Frustum()
{
}


Frustum::~Frustum()
{
}

inline const Frustum::FrustumPlane * Frustum::getPlanes() 
{
	return mPlanes;
}

inline void Frustum::constructFrustum(float3 camPos, float3 camDir, float3 up, float fowAngle, float aspectRatio, float nearZ, float farZ)
{
	camDir.Normalize();
	float3 middleFar = camPos + camDir * farZ;
	float3 middleNear = camPos + camDir * nearZ;

	float3 vectorLeft = camDir.Cross(up);
	vectorLeft.Normalize();
	float3 vectorDown = camDir.Cross(vectorLeft);
	vectorDown.Normalize();

	float halfHeightFar = farZ * tan(fowAngle / 2);
	float halfWidthFar = halfHeightFar * aspectRatio;

	float halfHeightNear = nearZ * tan(fowAngle / 2);
	float halfWidthNear = halfHeightNear * aspectRatio;

	float3 pointLeftUpFar		= middleFar + vectorLeft * halfWidthFar - vectorDown * halfHeightFar;
	float3 pointRightUpFar		= middleFar - vectorLeft * halfWidthFar - vectorDown * halfHeightFar;
	float3 pointLeftBottomFar	= middleFar + vectorLeft * halfWidthFar + vectorDown * halfHeightFar;
	float3 pointRightBottomFar	= middleFar - vectorLeft * halfWidthFar + vectorDown * halfHeightFar;

	mPlanes[Near].mPoint = middleNear;
	mPlanes[Near].mNormal = vectorLeft.Cross(vectorDown);

	mPlanes[Left].mPoint = middleNear + vectorLeft * halfWidthNear;
	mPlanes[Left].mNormal = vectorDown.Cross(mPlanes[Left].mPoint - pointLeftUpFar);
	
	mPlanes[Right].mPoint = middleNear - vectorLeft * halfWidthNear;
	mPlanes[Right].mNormal = vectorDown.Cross(pointRightBottomFar - mPlanes[Right].mPoint);
	
	mPlanes[Far].mPoint = middleFar;
	mPlanes[Far].mNormal = vectorDown.Cross(pointLeftUpFar - pointRightUpFar);
	
	mPlanes[Top].mPoint = middleNear - vectorDown * halfHeightNear;
	mPlanes[Top].mNormal = (pointRightUpFar - mPlanes[Top].mPoint).Cross(pointLeftUpFar - mPlanes[Top].mPoint);
	
	mPlanes[Bottom].mPoint = middleNear + vectorDown * halfHeightNear;
	mPlanes[Bottom].mNormal = (pointLeftBottomFar - mPlanes[Bottom].mPoint).Cross(pointRightBottomFar - mPlanes[Bottom].mPoint);

	for (int i = 0; i < 6; i++)
	{
		mPlanes[i].mNormal.Normalize();
	}
}
