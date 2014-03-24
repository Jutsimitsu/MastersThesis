#include "SkeletalData.h"


SkeletalData::SkeletalData(void)
{
	this->bonesCount = 0;
}


SkeletalData::~SkeletalData(void)
{
}

SkeletalData::SkeletalData(const SkeletalData &copyData) : bones(copyData.bones), bonesCount(copyData.bonesCount)
{
}
