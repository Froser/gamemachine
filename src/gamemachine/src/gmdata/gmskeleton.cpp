#include "stdafx.h"
#include <gmskeleton.h>

GMSkeletonJoint::~GMSkeletonJoint()
{
	removeAll();
}

void GMSkeletonJoint::removeAll()
{
	D(d);
	removeJoint(this);
}

void GMSkeletonJoint::removeJoint(GMSkeletonJoint* joint)
{
	D(d);
	// 先遍历递归删除所有children
	for (auto child : d->children)
	{
		removeJoint(child);
		GM_delete(child);
	}
	d->children.clear();
}

void GMSkeleton::setRootJoint(const GMSkeletonJoint& joint)
{
	D(d);
	d->rootJoint.reset(new GMSkeletonJoint(joint));
}