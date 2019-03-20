#pragma once
#include "standardClasses.h"

struct SortItem
{
	int mIndex;
	float mDistance;
	
	SortItem(float dist = -1, int index = -1)
	{
		mIndex = index;
		mDistance = dist;
	}
	bool operator<(const SortItem& other) 
	{
		return mDistance < other.mDistance;
	}
	bool operator>(const SortItem& other)
	{
		return mDistance > other.mDistance;
	}
	
};

class FrontToBack
{

private:
	Array<SortItem> mArray;

public:
	FrontToBack();
	~FrontToBack();
	void resetArray();
	void insert(float dist, int index);
	Array<int> getSortedIndexArray();
};



FrontToBack::FrontToBack()
{
}


FrontToBack::~FrontToBack()
{
}

inline void FrontToBack::resetArray()
{
	mArray.reset();
}

inline void FrontToBack::insert(float dist, int index)
{
	mArray.add(SortItem(dist, index));
}

inline Array<int> FrontToBack::getSortedIndexArray()
{
	Array<int> returnArray;
	mArray.quickSort();
	for (int i = 0; i < mArray.length(); i++)
	{
		returnArray.add(mArray[i].mIndex);
	}
	return returnArray;
}
