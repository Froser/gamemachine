#ifndef __GMSKELETON_H__
#define __GMSKELETON_H__
#include <gmcommon.h>
BEGIN_NS

GM_ALIGNED_STRUCT(GMBaseFrame)
{
	GMVec3 position;
	GMQuat orientation;
};

class GMSkeletonJoint;
GM_PRIVATE_OBJECT(GMSkeletonJoint)
{
	GMint parentIndex = -1;
	GMVec3 position = Zero<GMVec3>();
	GMQuat orientation = Identity<GMQuat>();
};

class GMSkeletonJoint : public GMObject
{
	GM_DECLARE_PRIVATE(GMSkeletonJoint)
	GM_ALLOW_COPY_DATA(GMSkeletonJoint)
	GM_DECLARE_PROPERTY(Position, position, GMVec3)
	GM_DECLARE_PROPERTY(Orientation, orientation, GMQuat)
	GM_DECLARE_PROPERTY(ParentIndex, parentIndex, GMint)

public:
	enum
	{
		RootIndex = -1,
	};

public:
	GMSkeletonJoint() = default;
};

//> 每一帧所有的关节状态
GM_PRIVATE_OBJECT(GMFrameSkeleton)
{
	Vector<GMSkeletonJoint> joints;
};

class GMFrameSkeleton : public GMObject
{
	GM_DECLARE_PRIVATE(GMFrameSkeleton)
	GM_ALLOW_COPY_DATA(GMFrameSkeleton)
	GM_DECLARE_PROPERTY(Joints, joints, Vector<GMSkeletonJoint>)

public:
	GMFrameSkeleton() = default;
};

//> 所有帧的关节状态
class GMFrameSkeletons : public Vector<GMFrameSkeleton>
{
public:
	inline GMsize_t getNumFrames() GM_NOEXCEPT
	{
		return size();
	}
};

GM_PRIVATE_OBJECT(GMSkeleton)
{
	GMFrameSkeletons skeletons;
	GMFrameSkeleton animatedSkeleton;
	GMfloat frameRate = 60;
};

class GMSkeleton : public GMObject
{
	GM_DECLARE_PRIVATE(GMSkeleton)
	GM_DECLARE_PROPERTY(Skeletons, skeletons, GMFrameSkeletons)
	GM_DECLARE_PROPERTY(AnimatedSkeleton, animatedSkeleton, GMFrameSkeleton)
	GM_DECLARE_PROPERTY(FrameRate, frameRate, GMfloat)

public:
	void interpolateSkeletons(GMint frame0, GMint frame1, GMfloat p);
};

END_NS
#endif