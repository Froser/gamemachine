#ifndef __GMSKELETON_H__
#define __GMSKELETON_H__
#include <gmcommon.h>
BEGIN_NS

GM_ALIGNED_STRUCT(GMSkeletonVertex)
{
	GMVec2 texCoords = Zero<GMVec2>();
	GMint startWeight;
	GMint weightCount;
};

GM_ALIGNED_STRUCT(GMSkeletonWeight)
{
	GMint jointIndex;
	GMfloat weightBias;
	GMVec3 weightPosition;
};

GM_ALIGNED_STRUCT(GMSkeletonMesh)
{
	GMString shader;
	GMint numVertices;
	GMint numTriangles;
	GMint numWeights;
	AlignedVector<Array<GMint, 3>> triangleIndices;
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
	GMint parentIndex = -1;
	GMVec3 position = Zero<GMVec3>();
	GMQuat orientation = Identity<GMQuat>();
};

class GMSkeletonJoint : public GMObject
{
	GM_DECLARE_PRIVATE(GMSkeletonJoint)
	GM_ALLOW_COPY_MOVE(GMSkeletonJoint)
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
	void interpolateSkeletons(GMint frame0, GMint frame1, GMfloat p);
};

END_NS
#endif