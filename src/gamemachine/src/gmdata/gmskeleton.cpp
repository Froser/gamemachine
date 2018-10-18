#include "stdafx.h"
#include <gmskeleton.h>

void GMSkeleton::interpolateSkeletons(GMint32 frame0, GMint32 frame1, GMfloat p)
{
	// 变换每个Joint
	D(d);
	GMSkeletonJoint jointResult;
	for (GMsize_t i = 0; i < d->animatedSkeleton.getJoints().size(); ++i)
	{
		GMSkeletonJoint& finalJoint = d->animatedSkeleton.getJoints()[i];
		const GMSkeletonJoint& joint0 = d->skeletons[frame0].getJoints()[i];
		const GMSkeletonJoint& joint1 = d->skeletons[frame1].getJoints()[i];
		finalJoint.setParentIndex(joint0.getParentIndex());
		finalJoint.setPosition(Lerp(joint0.getPosition(), joint1.getPosition(), p));
		finalJoint.setOrientation(Lerp(joint0.getOrientation(), joint1.getOrientation(), p));
	}
}

GMSkeletalAnimationEvaluator::GMSkeletalAnimationEvaluator(GMSkeletalAnimation* animation)
{
	D(d);
	d->animation = animation;
}

void GMSkeletalAnimationEvaluator::update(GMDuration dt)
{
	D(d);
	GM_ASSERT(d->animation);

	d->animationTime += dt;
	while (d->animationTime > d->animation->duration)
	{
		d->animationTime -= d->animation->duration;
	}

	while (d->animationTime < 0.0f)
	{
		d->animationTime += d->animation->duration;
	}

	// 找到当前所在的相邻两帧
	GMfloat frameNum = d->animationTime * static_cast<GMfloat>(d->animation->frameRate);
	GMint32 frame0 = Floor(frameNum);
	GMint32 frame1 = Ceil(frameNum);

	for (const auto& joint : d->animation->joints)
	{
		if (!joint.positions.empty())
		{
			GMfloat frameIdx = frame0 % joint.positions.size();
			GMfloat nextFrameIdx = frame1 % joint.positions.size();
			const auto& frame = joint.positions[frameIdx];
			const auto& nextFrame = joint.positions[nextFrameIdx];
			GMDuration diffTime = nextFrame.time - frame.time;
		}
	}
}