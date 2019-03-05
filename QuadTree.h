#pragma once
#include "standardClasses.h"


class QuadTree
{
private:
	struct AABB 
	{
		float3 mCenterPos;
		float3 mHalfLength;

		bool containsAABB(const AABB& other) // Kanske inte kommer användas, raderas sen då
		{
			bool isInside = true;
			if (mCenterPos.x + mHalfLength.x < other.mCenterPos.x + other.mHalfLength.x || mCenterPos.x - mHalfLength.x > other.mCenterPos.x - other.mHalfLength.x) 
				isInside = false;
			else if (mCenterPos.z + mHalfLength.z < other.mCenterPos.z + other.mHalfLength.z || mCenterPos.z - mHalfLength.z > other.mCenterPos.z - other.mHalfLength.z) 
				isInside = false;
			return isInside;
		}

		bool intersectsAABB(const AABB& other)
		{
			bool intersects = true;
			if (mHalfLength.x + other.mHalfLength.x < abs(mCenterPos.x - other.mCenterPos.x) ) 
				intersects = false;
			else if (mHalfLength.z + other.mHalfLength.z < abs(mCenterPos.z - other.mCenterPos.z) ) 
				intersects = false;
			return intersects;
		}

		AABB(float3 pos, float3 halfLength)
		{
			mCenterPos = pos;
			mHalfLength = halfLength;
		} 
		AABB()
		{

		}
	};
	struct Obj {
		AABB mAABB;
		int mIndex;

		Obj(float3 centerPos, float3 halfLength, int index)
		{
			mAABB = AABB(centerPos, halfLength);
			mIndex = index;
		}
	};

	// Member variables
	QuadTree* mChildren[4];
	int mNrOfPartitions;		// Nr of times this node can be partitioned. Those with 0 are leaves.
	
	AABB mBoundingBox;
	Array<Obj> mObjects;

	// Help functions
	bool insert(const Obj& obj);
	void createChildren();
public:
	QuadTree(const AABB& boundingBox, int nrOfPartitions);
	~QuadTree();

	bool insert(const float3 centerPos, const float3 halfLength, const int index);
	 
	// lista ut hur du får ut index värdena på obj i frustumet. 
	// Idé 1:Ska frustum skickas in här och hanteras med en smart rekursiv funktion?
	// Idé 2:Ska jag skapa get aabb och obj.index funktioner och göra checkarna utanför? kanske i en frustum klass?
};

QuadTree::QuadTree(const AABB& boundingBox, int nrOfPartitions)
{
	for (int i = 0; i < 4; i++)
		mChildren[i] = nullptr;

	mBoundingBox = boundingBox;
	mNrOfPartitions = nrOfPartitions;	
}

QuadTree::~QuadTree()
{
	if (mChildren[0] != nullptr)
	{
		for (int i = 0; i < 4; i++)
		{
			delete mChildren[i];
		}
	}
}

inline bool QuadTree::insert(const Obj& obj)
{
	bool intersects = mBoundingBox.intersectsAABB(obj.mAABB);
	if (intersects)
	{
		if(mNrOfPartitions == 0) // If this node is a leaf
			mObjects.add(obj);
		else
		{
			if (mChildren[0] == nullptr)
				createChildren();

			for (int iChild = 0; iChild < 4; iChild++)
			{
				mChildren[iChild]->insert(obj);
			}
		}
	}
	return intersects;
}

inline bool QuadTree::insert(const float3 centerPos, const float3 halfLength, const int index)
{
	return insert(Obj(centerPos, halfLength, index));
}

inline void QuadTree::createChildren()
{
	float3 newHalfSizes = mBoundingBox.mHalfLength;
	newHalfSizes.x /= 2;
	newHalfSizes.z /= 2;	

	mChildren[0] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * float3(-1, 0,  1), newHalfSizes), mNrOfPartitions - 1);   //North West
	mChildren[1] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * float3( 1, 0,  1), newHalfSizes), mNrOfPartitions - 1);   //North East
	mChildren[2] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * float3(-1, 0, -1), newHalfSizes), mNrOfPartitions - 1);   //South West
	mChildren[3] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * float3( 1, 0, -1), newHalfSizes), mNrOfPartitions - 1);   //South East
}
