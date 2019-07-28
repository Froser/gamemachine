#ifndef __GMANIMATIONOBJECTHELPER_H__
#define __GMANIMATIONOBJECTHELPER_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

GM_DECLARE_POINTER(GMSkeleton);
GM_DECLARE_POINTER(GMNode);
typedef const GMNodeAnimation* GMNodeAnimationPtr;
GM_PRIVATE_CLASS(GMAnimationEvaluator);
class GMAnimationEvaluator
{
	GM_DECLARE_PRIVATE(GMAnimationEvaluator)
	GM_DISABLE_COPY_ASSIGN(GMAnimationEvaluator)
	GM_DECLARE_PROPERTY(GMSkeletonPtr, Skeleton)
	GM_DECLARE_PROPERTY(GMNodePtr, RootNode)
	GM_DECLARE_PROPERTY(GMNodeAnimationPtr, Animation)
	GM_DECLARE_GETTER_ACCESSOR(AlignedVector<GMMat4>, Transforms, public)

public:
	GMAnimationEvaluator(GMNode* root, GMSkeleton* skeleton);

public:
	void update(GMDuration dt);
	void reset();

private:
	void updateNode(GMDuration animationTime, GMNode* node, const GMMat4& parentTransformation);
	const GMNodeAnimationNode* findAnimationNode(const GMString& name);
};

GM_PRIVATE_CLASS(GMAnimationGameObjectHelper);
class GMAnimationGameObjectHelper
{
	GM_DECLARE_PRIVATE(GMAnimationGameObjectHelper)
	GM_DISABLE_COPY_ASSIGN(GMAnimationGameObjectHelper)
	GM_DECLARE_PROPERTY(GMVec4, SkeletonColor)

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
	bool isPlaying() GM_NOEXCEPT;
};

END_NS
#endif