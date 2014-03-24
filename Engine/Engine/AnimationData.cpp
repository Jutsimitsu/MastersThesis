#include "AnimationData.h"


AnimationData::AnimationData(void)
{
}


AnimationData::~AnimationData(void)
{
}

AnimationData::AnimationData(char* name, double start, double end, int fps, int fr)
{
	this->takeName = name;
	this->startTime = start;
	this->endTime = end;
	this->fps = fps;

	allFrames = fr;
	frames = new SkeletalData*[fr];
}