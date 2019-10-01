#include "Boat.h"



void Boat::calculateThust()
{
}

float Boat::speedOfAdvance()
{
	return (1 - realSlipRatio()) * mPitch * (mGas * mMaxTurnOverRate);
}

float Boat::realSlipRatio()
{
	return (mPitch -mPitchWater)/mPitchWater;
}

Boat::Boat(Mesh* mesh)
{
	mBoat.giveMesh(mesh);
	mVDir = float3(0, 0, 0);
	mCameraAnchor = float3(0, 3, -3);
	mCannonStandAnchor = float3(0, 0, 0.5);
	mSpeed = 1;
}

Boat::Boat()
{
}


Boat::~Boat()
{
}

void Boat::update(float dt, float2 inputDir)
{
	mBoat.rotateY(inputDir.x * 45 * dt);
	mBoat.move(float3(0, 0, inputDir.y*mSpeed*dt));
}
