#pragma once

#include "SkeletalData.h"
#include "AnimationData.h"
#include <iostream>
#include <vector>

using namespace std;

class AnimationFrame
{
private:
	AnimationData *animationDat;

	int ID;
	SkeletalData *pose;

public:
	AnimationFrame(void);
	~AnimationFrame(void);
	AnimationFrame(AnimationData*);

	void SetID (int id) { ID = id; }
	void SetPose (SkeletalData *skeleton) { pose = new SkeletalData(*skeleton); }
	void AddAnimation (AnimationData *dat) { animationDat = dat; }

	AnimationData* GetData() { return animationDat; }

	SkeletalData GetPose() { return *pose; }
};

