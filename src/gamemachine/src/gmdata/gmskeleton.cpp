#include "stdafx.h"
#include <gmskeleton.h>

void GMSkeleton::interpolateSkeletons(GMint frame0, GMint frame1, GMfloat p)
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