#ifndef __GMSKELETON_H__
#define __GMSKELETON_H__
#include <gmcommon.h>
BEGIN_NS

class GMSkeletalAnimationEvaluator;
struct GMSceneAnimatorNode;
struct GMSkeletalAnimation;

struct GMSkeletonWeight
{
	GMuint32 vertexId;
	GMfloat weight;
};

GM_ALIGNED_STRUCT(GMSkeletalBone)
{
	GMString name;
	GMMat4 offsetMatrix; //!< 从模型空间到绑定姿势的变换
	GMModel* targetModel = nullptr;
	Vector<GMSkeletonWeight> weights;
	GMMat4 finalTransformation; //!< 所有的计算结果将会放到这里来
};

GM_PRIVATE_OBJECT(GMSkeletalNode)
{
	GMString name;
	GMSkeletalNode* parent = nullptr;
	Vector<GMSkeletalNode*> children;
	GMMat4 transformToParent;
};

GM_ALIGNED_16(class) GMSkeletalNode
{
	GM_DECLARE_PRIVATE_NGO(GMSkeletalNode)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Name, name)
	GM_DECLARE_PROPERTY(Parent, parent)
	GM_DECLARE_PROPERTY(Children, children)
	GM_DECLARE_PROPERTY(TransformToParent, transformToParent)

public:
	~GMSkeletalNode()
	{
		for (auto& child : getChildren())
		{
			GM_delete(child);
		}
	}
};

GM_PRIVATE_OBJECT_UNALIGNED(GMSkeletalVertexBoneData)
{
	enum
	{
		BonesPerVertex = 4 // 每个vertex最大的bone数目
	};

	GMsize_t ids[BonesPerVertex] = { 0 }; //!< 定点绑定的骨骼索引
	GMfloat weights[BonesPerVertex] = { 0 };
};

class GMSkeletalVertexBoneData
{
public:
	enum
	{
		BonesPerVertex = GM_PRIVATE_NAME(GMSkeletalVertexBoneData)::BonesPerVertex // 每个vertex最大的bone数目
	};

	GM_DECLARE_PRIVATE_NGO(GMSkeletalVertexBoneData)
	
public:
	GMSkeletalVertexBoneData() = default;

public:
	void reset();
	void addData(GMsize_t boneId, GMfloat weight);

public:
	inline GMsize_t* getIds() GM_NOEXCEPT
	{
		D(d);
		return d->ids;
	}

	inline GMfloat* getWeights() GM_NOEXCEPT
	{
		D(d);
		return d->weights;
	}
};

GM_PRIVATE_OBJECT(GMSkeletalBones)
{
	AlignedVector<GMSkeletalBone> bones;
	Vector<GMSkeletalVertexBoneData> vertexData;
	Map<GMString, GMsize_t> boneNameIndexMap;
};

class GMSkeletalBones
{
	GM_DECLARE_PRIVATE_NGO(GMSkeletalBones)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Bones, bones)
	GM_DECLARE_PROPERTY(BoneNameIndexMap, boneNameIndexMap)
	GM_DECLARE_PROPERTY(VertexData, vertexData)
};

GM_PRIVATE_OBJECT(GMSkeleton)
{
	GMSkeletalBones bones;
};

class GMSkeleton
{
	GM_DECLARE_PRIVATE_NGO(GMSkeleton)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Bones, bones)
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

GM_ALIGNED_STRUCT(GMSkeletalAnimationNode)
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
	AlignedVector<GMSkeletalAnimationNode> nodes;
};

GM_PRIVATE_OBJECT(GMSkeletalAnimations)
{
	AlignedVector<GMSkeletalAnimation> animations;
};

GM_ALIGNED_16(class) GMSkeletalAnimations
{
	GM_DECLARE_PRIVATE_NGO(GMSkeletalAnimations)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Animations, animations)

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

END_NS
#endif