#pragma once
#include "standardClasses.h"
#include "Frustum.h"

class QuadTree
{
private:
	struct AABB 
	{
		float3 mCenterPos;
		float3 mHalfLength;

		bool containsAABB(const AABB& other) // Kanske inte kommer anv�ndas, raderas sen d�
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
		Obj(){}
	};
	enum IntersectType{Outside, Intersects, Inside};
	
	// Member variables
	QuadTree* mChildren[4];
	int mNrOfPartitions;		// Nr of times this node can be partitioned. Those with 0 are leaves.
	
	AABB mBoundingBox;
	Array<Obj> mObjects;

	float3 mPoints[8];
	float3 mDiagonals[4];

	// Help functions
	bool insert(const Obj& obj);
	void createChildren();
	
	int intersectsFrustum(Frustum frustum);			
	void getContent(Array<int>& indexArray);
	void getAllContentFromLeaves(Array<int>& indexArray);
	void setUpPointsAndDiagonals();
public:
	QuadTree(const AABB& boundingBox, int nrOfPartitions);
	QuadTree(float3 centerPos, float3 halfLengths, int nrOfPartitions);
	~QuadTree();

	bool insert(const float3 centerPos, const float3 halfLength, const int index);
	
	void checkagainstFrustum(Array<int>& indexArray, Frustum frustum);

};

QuadTree::QuadTree(const AABB& boundingBox, int nrOfPartitions)
{
	for (int i = 0; i < 4; i++)
		mChildren[i] = nullptr;

	mBoundingBox = boundingBox;
	mNrOfPartitions = nrOfPartitions;	
	setUpPointsAndDiagonals();
}

inline QuadTree::QuadTree(float3 centerPos = float3(0, 0, 0), float3 halfLengths = float3(1, 1, 1), int nrOfPartitions = 3)
	:QuadTree(AABB(centerPos, halfLengths), nrOfPartitions)
{/*
	for (int i = 0; i < 4; i++)
		mChildren[i] = nullptr;

	mBoundingBox = AABB(centerPos, halfLengths);
	mNrOfPartitions = nrOfPartitions;
	setUpPointsAndDiagonals();*/
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

inline void QuadTree::checkagainstFrustum(Array<int>& indexArray, Frustum frustum)
{
	/*
	if ( leaf)
		if(intersects)
			getContent

	else if(contained)
		getAllContent()

	else if(intersects)
		children.checkagainstFrustum()	
	*/

	int intersects = intersectsFrustum(frustum);


	if (mChildren[0] == nullptr)
	{
		if (intersects > 0)
		{
			getContent(indexArray);
		}
	}
	else if (intersects == Inside)
	{
		getAllContentFromLeaves(indexArray);
	}
	else if (intersects == Intersects)
	{
		for (int i = 0; i < 4; i++)
		{
			mChildren[i]->checkagainstFrustum(indexArray, frustum);
		}
	}
}

inline void QuadTree::createChildren()
{
	float3 newHalfSizes = mBoundingBox.mHalfLength;
	newHalfSizes.x /= 2;
	newHalfSizes.z /= 2;	

	//mChildren[0] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * float3(-1, 0,  1), newHalfSizes), mNrOfPartitions - 1);   //North West
	//mChildren[1] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * float3( 1, 0,  1), newHalfSizes), mNrOfPartitions - 1);   //North East
	//mChildren[2] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * float3(-1, 0, -1), newHalfSizes), mNrOfPartitions - 1);   //South West
	//mChildren[3] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * float3( 1, 0, -1), newHalfSizes), mNrOfPartitions - 1);   //South East
	// Tror den nedan �r mer l�ttl�st?
	  
	float3 direction[4] = { float3(-1, 0, 1), float3(1, 0, 1) ,float3(-1, 0, -1), float3(1, 0, -1) };
	for (int i = 0; i < 4; i++) 
		mChildren[i] = new QuadTree(AABB(mBoundingBox.mCenterPos + newHalfSizes * direction[i], newHalfSizes), mNrOfPartitions - 1);
}


inline int QuadTree::intersectsFrustum(Frustum frustum)		
{
	// returns 0: outside, 1: intersects and 2: contained
	
	int returnValue = 2;
	float similarity;
	float newSimilarity;
	int mostSimilarDiagonal;
	for (int iPlane = 0; iPlane < 6 && returnValue > 0; iPlane++)
	{
		similarity = 0;
		newSimilarity = 0;
		mostSimilarDiagonal = -1;
		for (int iDiagonal = 0; iDiagonal < 4; iDiagonal++)
		{
			newSimilarity = abs(frustum.getPlanes()[iPlane].mNormal.Dot(mDiagonals[iDiagonal]));
			if (newSimilarity > similarity)
			{
				similarity = newSimilarity;
				mostSimilarDiagonal = iDiagonal;
			}
		}

		// Distance taken from the length of a vector (from the plane to a point) projected onto the plane normal.
		// Think the projection formula but without the resulting vector.
		// The normals are normalized.
		// Normals point inwards. 
		float p = (mPoints[mostSimilarDiagonal * 2] - frustum.getPlanes()[iPlane].mPoint).Dot(frustum.getPlanes()[iPlane].mNormal);
		float n = (mPoints[mostSimilarDiagonal * 2 + 1] - frustum.getPlanes()[iPlane].mPoint).Dot(frustum.getPlanes()[iPlane].mNormal);

		if (p < n)
		{
			float temp = p;
			p = n;
			n = temp;
		}

		if (p < 0)
			returnValue = Outside;
		else if (n < 0)
			returnValue = Intersects;
	}
	return returnValue;
}

inline void QuadTree::getContent(Array<int>& indexArray)
{
	int nr = indexArray.length();

	bool isInArray = false;
	for (int i = 0; i < mObjects.length(); i++)
	{
		isInArray = false;
		for (int iArray = 0; iArray < indexArray.length() && isInArray == false; iArray++)
		{
			if (indexArray.get(iArray) == mObjects[i].mIndex)
				isInArray = true;
		}
		if(!isInArray)
			indexArray.add(mObjects.get(i).mIndex);
	}

}

inline void QuadTree::getAllContentFromLeaves(Array<int>& indexArray)
{
	if (mChildren[0] != nullptr)
	{
		for (int i = 0; i < 4; i++)
		{
			mChildren[i]->getAllContentFromLeaves(indexArray);
		}
	}
	else
	{
		getContent(indexArray);
	}
}

inline void QuadTree::setUpPointsAndDiagonals()
{
	//index 2n and 2n+1 is a pair
	
	//points[0] = mBoundingBox.mCenterPos + mBoundingBox.mHalfLength;
	//points[1] = mBoundingBox.mCenterPos - mBoundingBox.mHalfLength;
	//
	//points[2] = mBoundingBox.mCenterPos + mBoundingBox.mHalfLength * float3(-1, -1,  1);
	//points[3] = mBoundingBox.mCenterPos + mBoundingBox.mHalfLength * float3( 1,  1, -1);
	//
	//points[4] = mBoundingBox.mCenterPos + mBoundingBox.mHalfLength * float3(-1,  1, -1);
	//points[5] = mBoundingBox.mCenterPos + mBoundingBox.mHalfLength * float3( 1, -1,  1);
	//
	//points[6] = mBoundingBox.mCenterPos + mBoundingBox.mHalfLength * float3( 1, -1, -1);
	//points[7] = mBoundingBox.mCenterPos + mBoundingBox.mHalfLength * float3(-1,  1,  1);

	float3 direction[4] = { float3(1, 1, 1), float3(-1, 1, 1) ,float3(1, -1, 1), float3(1, 1, -1) };
	for (int i = 0; i < 4; i++)
	{
		mPoints[2 * i] = mBoundingBox.mCenterPos + mBoundingBox.mHalfLength * direction[i];
		mPoints[2 * i + 1] = mBoundingBox.mCenterPos - mBoundingBox.mHalfLength * direction[i];
	}

	for (int i = 0; i < 4; i++)
	{
		mDiagonals[i] = mPoints[2 * i] - mPoints[2 * i + 1];
		mDiagonals[i].Normalize();
	}
}
