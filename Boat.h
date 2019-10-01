#pragma once
#include "Object.h"
class Boat: private Object	// or player
{
private:
	Object mBoat;	//kanske ha pekare o riktiga obj i arrayen i main?
					//eller kanske ha arrayen i main inneh�lla pekare					
	float3 mVDir;
	float3 mCameraAnchor;
	float3 mCannonStandAnchor;


private:
	//float mSpeed;
	
	float mTurnAngle; //Angle of Propellar
	float3 mVelocity;
	float3 mRotation;
	const float mPitch,mPitchWater ; //emperisk
	const float mHullEfficiency; //tecknet "ny" i boken

	float mGas; // 0 <= mGas <= 1;
	const float mMaxTurnOverRate; //propellerfrekvens vid full gas
	void calculateThust();

	float speedOfAdvance();
	float realSlipRatio(); //G�r denna till const.
	 
public:
	Boat(Mesh* mesh);
	Boat();
	~Boat();

	void update(float dt, float2 inputDir);   // input tempor�r
};

