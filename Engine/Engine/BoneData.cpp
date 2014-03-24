#include "BoneData.h"


BoneData::BoneData(void)
{
	this->id = 0;
	this->localOrientation = D3DXQUATERNION(0,0,0,0);
	this->localPosition = D3DXVECTOR3(0,0,0);
	this->name = "";
	this->parentID = NULL;
}

BoneData::~BoneData(void)
{
}

BoneData::BoneData(const BoneData &copyData) : id(copyData.id), localOrientation(copyData.localOrientation), localPosition(copyData.localPosition), name(copyData.name), parentID(copyData.parentID)
{
}
