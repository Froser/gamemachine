#ifndef __GMANIMATIONOBJECTHELPER_H__
#define __GMANIMATIONOBJECTHELPER_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMAnimationEvaluator)
{
	const GMNodeAnimation* animation = nullptr;
	GMDuration duration = 0;
	AlignedVector<GMMat4> transforms;
	GMSkeleton* skeleton = nullptr;
	GMNode* rootNode = nullptr;
	GMMat4 globalInverseTransform;
	GMfloat lastTime = 0;
	Map<const GMNodeAnimationNode*, Array<GMint32, 3>> lastFrameIdx;
};

class GMAnimationEvaluator
{
	GM_DECLARE_PRIVATE_NGO(GMAnimationEvaluator)
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PROPERTY(Skeleton, skeleton)
	GM_DECLARE_PROPERTY(RootNode, rootNode)
	GM_DECLARE_PROPERTY(Animation, animation)
	GM_DECLARE_GETTER(Transforms, transforms)

public:
	GMAnimationEvaluator(GMNode* root, GMSkeleton* skeleton);

public:
	void update(GMDuration dt);
	void reset();

private:
	void updateNode(GMDuration animationTime, GMNode* node, const GMMat4& parentTransformation);
	const GMNodeAnimationNode* findAnimationNode(const GMString& name);
};

GM_PRIVATE_OBJECT(GMAnimationGameObjectHelper)
{
	enum { AutoPlayFrame = -1 };

	bool playing = true;
	GMGameObject* host = nullptr;
	GMVec4 skeletonColor = GMVec4(0, 1, 0, 1);
	Map<GMModel*, GMAnimationEvaluator*> modelEvaluatorMap;
	Vector<GMString> nameList;
	GMsize_t animationIndex = 0;
};

class GMAnimationGameObjectHelper
{
	GM_DECLARE_ALIGNED_ALLOCATOR()
	GM_DECLARE_PRIVATE_NGO(GMAnimationGameObjectHelper)
	GM_DECLARE_PROPERTY(SkeletonColor, skeletonColor)

public:
	enum { AutoPlayFrame = -1 };
	GMAnimationGameObjectHelper(GMGameObject* host);
	~GMAnimationGameObjectHelper();

public:
	virtual void update(GMDuration dt);

public:
	void play();
	void pause();
	void reset(bool update);

	GMsize_t getAnimationCount();
	void setAnimation(GMsize_t index);
	Vector<GMString> getAnimationNames();
	GMsize_t getAnimationIndexByName(const GMString& name);

public:
	inline bool isPlaying() GM_NOEXCEPT
	{
		D(d);
		return d->playing;
	}
};

END_NS
#endif