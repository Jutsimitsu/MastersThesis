#pragma once

#include "BoneData.h"
#include <vector>

using namespace std;

class SkeletalData
{
private:
	int bonesCount;
	std::vector<BoneData*> bones;
public:
	SkeletalData(void);
	SkeletalData(const SkeletalData&);
	~SkeletalData(void);

	int GetBonesCount () { return bonesCount; }
	BoneData* GetBone(int i) { return bones[i]; }
	int GetBoneByName(std::string _name) { 
		for (int i = 0; i< bonesCount; i++)
		{
			if (bones[i]->GetName().compare(_name) == 0)
				return bones[i]->GetID();

		}
		return 0;
	}	
	int GetBoneIndex(int i) {
		for(int j = 0; j<bonesCount; j++)
		{
			if (bones[j]->GetID() == i)
				return j;
		}
		return 0;
	}

	void SetBones (BoneData *_bones) { bonesCount++; bones.push_back(_bones); }
};

