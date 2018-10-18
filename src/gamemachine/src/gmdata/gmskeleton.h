#ifndef __GMSKELETON_H__
#define __GMSKELETON_H__
#include <gmcommon.h>
BEGIN_NS

GM_ALIGNED_STRUCT(GMSkeletonVertex)
{
	GMVec2 texCoords = Zero<GMVec2>();
	GMint32 startWeight;
	GMint32 weightCount;
};

GM_ALIGNED_STRUCT(GMSkeletonWeight)
{
	GMint32 jointIndex;
	GMfloat weightBias;
	GMVec3 weightPosition;
};

GM_ALIGNED_STRUCT(GMSkeletonMesh)
{
	GMString shader;
	GMint32 numVertices;
	GMint32 numTriangles;
	GMint32 numWeights;
	AlignedVector<Array<GMint32, 3>> triangleIndices;
	AlignedVector<GMSkeletonVertex> vertices;
	AlignedVector<GMSkeletonWeight> weights;
	GMModel* targetModel = nullptr;
};

GM_ALIGNED_STRUCT(GMBaseFrame)
{
	GMVec3 position;
	GMQuat orientation;
};

class GMSkeletonJoint;
GM_PRIVATE_OBJECT(GMSkeletonJoint)
{
	GMint32 parentIndex = -1;
	GMVec3 position = Zero<GMVec3>();
	GMQuat orientation = Identity<GMQuat>();
};

class GMSkeletonJoint : public GMObject
{
	GM_DECLARE_PRIVATE(GMSkeletonJoint)
	GM_ALLOW_COPY_MOVE(GMSkeletonJoint)
	GM_DECLARE_PROPERTY(Position, position, GMVec3)
	GM_DECLARE_PROPERTY(Orientation, orientation, GMQuat)
	GM_DECLARE_PROPERTY(ParentIndex, parentIndex, GMint32)

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
	AlignedVector<GMSkeletonJoint> joints;
};

class GMFrameSkeleton : public GMObject
{
	GM_DECLARE_PRIVATE(GMFrameSkeleton)
	GM_ALLOW_COPY_MOVE(GMFrameSkeleton)
	GM_DECLARE_PROPERTY(Joints, joints, AlignedVector<GMSkeletonJoint>)

public:
	GMFrameSkeleton() = default;
};

//> 所有帧的关节状态
class GMFrameSkeletons : public AlignedVector<GMFrameSkeleton>
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
	AlignedVector<GMSkeletonMesh> meshes;
	GMfloat frameRate = 60;
};

class GMSkeleton : public GMObject
{
	GM_DECLARE_PRIVATE(GMSkeleton)
	GM_DECLARE_PROPERTY(Skeletons, skeletons, GMFrameSkeletons)
	GM_DECLARE_PROPERTY(AnimatedSkeleton, animatedSkeleton, GMFrameSkeleton)
	GM_DECLARE_PROPERTY(FrameRate, frameRate, GMfloat)
	GM_DECLARE_PROPERTY(Meshes, meshes, AlignedVector<GMSkeletonMesh>)

public:
	void interpolateSkeletons(GMint32 frame0, GMint32 frame1, GMfloat p);
};


//////////////////////////////////////////////////////////////////////////
template <typename T>
struct GMSkeletonAnimationKeyframe
{
	GMSkeletonAnimationKeyframe(GMDuration t, T&& v)
		: time(t)
		, value(v)
	{
	}

	GMDuration time;
	T value;
};

GM_ALIGNED_STRUCT(GMSkeletalAnimationJoint)
{
	GMString name;
	AlignedVector<GMSkeletonAnimationKeyframe<GMVec3>> positions;
	AlignedVector<GMSkeletonAnimationKeyframe<GMVec3>> scalings;
	AlignedVector<GMSkeletonAnimationKeyframe<GMQuat>> rotations;
};

GM_ALIGNED_STRUCT(GMSkeletalAnimation)
{
	GMfloat frameRate = 25;
	GMDuration duration;
	AlignedVector<GMSkeletalAnimationJoint> joints;
};

GM_PRIVATE_OBJECT(GMSkeletalAnimations)
{
	AlignedVector<GMSkeletalAnimation> animations;
};

GM_ALIGNED_16(class) GMSkeletalAnimations
{
	GM_DECLARE_PRIVATE_NGO(GMSkeletalAnimations)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Animations, animations, AlignedVector<GMSkeletalAnimation>)

public:
	inline GMSkeletalAnimation* getAnimation(GMsize_t index) GM_NOEXCEPT
	{
		D(d);
		return &d->animations[index];
	}

	inline GMsize_t getAnimationCount()
	{
		D(d);
		return d->animations.size();
	}
};

GM_PRIVATE_OBJECT_UNALIGNED(GMSkeletalAnimationEvaluator)
{
	GMSkeletalAnimation* animation = nullptr;
	GMDuration animationTime = 0;
	GMMat4 transform = Identity<GMMat4>();
};

class GMSkeletalAnimationEvaluator
{
	GM_DECLARE_PRIVATE_NGO(GMSkeletalAnimationEvaluator)

public:
	GMSkeletalAnimationEvaluator(GMSkeletalAnimation* animation);

public:
	void update(GMDuration dt);
};

END_NS
#endif