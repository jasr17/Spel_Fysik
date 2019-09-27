#pragma once
#include "Object.h"
class Boat		// or player
{
private:
	Object mBoat;	//kanske ha pekare o riktiga obj i arrayen i main?
					//eller kanske ha arrayen i main innehålla pekare					
	float3 mVDir;
	float3 mCameraAnchor;
	float3 mCannonStandAnchor;
	float mSpeed;


public:
	Boat(Mesh* mesh);
	Boat();
	~Boat();

	void update(float dt, float2 inputDir);   // input temporär
};

