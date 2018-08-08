#include "stdafx.h"
#include "gmskeletongameobject.h"

void GMSkeletonGameObject::update(GMDuration dt)
{
	D(d);
	GMModels& models = getModels();
	auto skeleton = models.getSkeleton();
	if (skeleton)
	{
		initAnimation();
		GMint frame0 = 0, frame1 = 0;
		GMfloat interpolate = 0;
		getAdjacentTwoFrames(dt, frame0, frame1, interpolate);
	}
}

void GMSkeletonGameObject::initAnimation()
{
	D(d);
	GMSkeleton* skeleton = getModels().getSkeleton();
	GM_ASSERT(skeleton);
	if (d->frameDuration == 0)
	{
		d->frameDuration = 1.f / skeleton->getFrameRate();
	}

	if (d->animationDuration == 0)
	{
		// 计算播放全部动画需要的时间
		d->animationDuration = d->frameDuration * skeleton->getSkeletons().getNumFrames();
	}
}

void GMSkeletonGameObject::getAdjacentTwoFrames(GMDuration dt, REF GMint& frame0, REF GMint& frame1, REF GMfloat& interpolate)
{
	D(d);
	GMSkeleton* skeleton = getModels().getSkeleton();
	GM_ASSERT(skeleton);
	if (skeleton->getSkeletons().getNumFrames() < 1)
		return;

	d->animationTime += dt;
	while (d->animationTime > d->animationDuration)
	{
		d->animationTime -= d->animationDuration;
	}

	while (d->animationTime < 0.0f)
	{
		d->animationTime += d->animationDuration;
	}

	// 找到当前所在的帧
	GMfloat frameNum = d->animationTime * static_cast<GMfloat>(skeleton->getFrameRate());
	frame0 = Floor(frameNum);
	frame1 = Ceil(frameNum);
	frame0 = frame0 % skeleton->getSkeletons().getNumFrames();
	frame1 = frame1 % skeleton->getSkeletons().getNumFrames();

	interpolate = Fmod(d->animationTime, d->frameDuration) / d->frameDuration;
}