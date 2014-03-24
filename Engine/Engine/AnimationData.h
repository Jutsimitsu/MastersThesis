#pragma once

#include "SkeletalData.h"
#include "BoneData.h"
#include <iostream>
#include <vector>
#include <fbxsdk.h>

using namespace std;

class AnimationData
{
private:
	char* takeName;
	double startTime;
	double endTime;
	int fps;
	int allFrames;

	struct weightsStructure
	{
		int boneID;
		int vertexIndex;
		double weight;
	} ;

	std::vector<weightsStructure> weights;

	SkeletalData** frames;

public:
	AnimationData(void);
	AnimationData(char*, double, double, int, int);
	~AnimationData(void);
	
	void AddWeight(int bone, int index, double weight) {
		weightsStructure weightToAdd;
		weightToAdd.boneID = bone;
		weightToAdd.vertexIndex = index;
		weightToAdd.weight = weight;
		weights.push_back(weightToAdd); } ;

	void SetSkeleton(SkeletalData* data, int i) { frames[i] = new SkeletalData(*data); }
		
	SkeletalData* GetSkeleton(int i) { return frames[i]; }

	int GetFramesNumber(){return allFrames;}
	FbxTime getStartTime() { return FbxTime(startTime); }
	FbxTime getEndTime(){return FbxTime(endTime); }
	FbxTime getDeltaTime(){return FbxTime((endTime-startTime)/(double)allFrames);}
};

