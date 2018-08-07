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
	GMVec3 position;
	GMQuat orientation;
	GMSkeletonJoint* parent;
	Vector<GMSkeletonJoint*> children;
};

class GMSkeletonJoint : public GMObject
{
	GM_DECLARE_PRIVATE(GMSkeletonJoint)
	GM_ALLOW_COPY_DATA(GMSkeletonJoint)
	GM_DECLARE_PROPERTY(Position, position, GMVec3)
	GM_DECLARE_PROPERTY(Orientation, orientation, GMQuat)

public:
	enum
	{
		RootId = -1,
	};

public:
	GMSkeletonJoint() = default;
	~GMSkeletonJoint();

private:
	void removeAll();
	void removeJoint(GMSkeletonJoint* joint);

public:
	inline const Vector<GMSkeletonJoint*>& getChildren() GM_NOEXCEPT
	{
		D(d);
		return d->children;
	}
};

//> 每一帧所有的关节状态
typedef Vector<GMSkeletonJoint> GMFrameSkeleton;

//> 所有帧的关节状态
typedef Vector<GMFrameSkeleton> GMFrameSkeletons;

GM_PRIVATE_OBJECT(GMSkeleton)
{
	GMOwnedPtr<GMSkeletonJoint> rootJoint;
	GMBaseFrame baseframe;
	GMFrameSkeletons frameSkeletons;
};

class GMSkeleton : public GMObject
{
	GM_DECLARE_PRIVATE(GMSkeleton)
	GM_DECLARE_PROPERTY(Baseframe, baseframe, GMBaseFrame)

public:
	void setRootJoint(const GMSkeletonJoint& joint);

public:
	inline GMSkeletonJoint* getRootJoint() GM_NOEXCEPT
	{
		D(d);
		return d->rootJoint.get();
	}
};

END_NS
#endif