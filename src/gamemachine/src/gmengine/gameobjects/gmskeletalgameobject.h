#ifndef __GMSKELETONGAMEOBJECT_H__
#define __GMSKELETONGAMEOBJECT_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMSkeletalAnimationEvaluator)
{
	const GMSkeletalAnimation* animation = nullptr;
	GMDuration duration = 0;
	AlignedVector<GMMat4> transforms;
	GMSkeleton* skeleton = nullptr;
	GMSkeletalNode* rootNode = nullptr;
	GMMat4 globalInverseTransform;
};

class GMSkeletalAnimationEvaluator
{
	GM_DECLARE_PRIVATE_NGO(GMSkeletalAnimationEvaluator)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Skeleton, skeleton)
	GM_DECLARE_PROPERTY(RootNode, rootNode)
	GM_DECLARE_PROPERTY(Animation, animation)
	GM_DECLARE_GETTER(Transforms, transforms)

public:
	GMSkeletalAnimationEvaluator(GMSkeletalNode* root, GMSkeleton* skeleton);

public:
	void update(GMDuration dt);
	void reset();

private:
	void updateNode(GMDuration animationTime, GMSkeletalNode* node, const GMMat4& parentTransformation);
	const GMSkeletalAnimationNode* findAnimationNode(const GMString& name);
};

GM_PRIVATE_OBJECT(GMSkeletalGameObject)
{
	enum { AutoPlayFrame = -1 };

	bool playing = true;
	GMVec4 skeletonColor = GMVec4(0, 1, 0, 1);
	Map<GMModel*, GMSkeletalAnimationEvaluator*> modelEvaluatorMap;
	GMsize_t animationIndex = 0;
};

class GMSkeletalGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMSkeletalGameObject, GMGameObject)
	GM_DECLARE_PROPERTY(SkeletonColor, skeletonColor)

public:
	enum { AutoPlayFrame = -1 };
	using GMGameObject::GMGameObject;
	~GMSkeletalGameObject();

public:
	virtual void update(GMDuration dt) override;

public:
	void createSkeletonBonesObject();
	void play();
	void pause();
	void reset(bool update);
	GMsize_t getAnimationCount();

public:
	inline bool isPlaying() GM_NOEXCEPT
	{
		D(d);
		return d->playing;
	}

private:
	void updateSkeleton();
};

END_NS
#endif