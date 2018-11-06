#ifndef __GMSKELETONGAMEOBJECT_H__
#define __GMSKELETONGAMEOBJECT_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMSkeletalGameObject)
{
	enum { AutoPlayFrame = -1 };

	GMDuration animationTime = 0; // deprecate soon
	GMDuration animationDuration = 0; // deprecate soon
	GMDuration frameDuration = 0; // deprecate soon
	GMint32 frame = AutoPlayFrame;
	GMint32 frameInterpolate = 0;
	GMOwnedPtr<GMGameObject> skeletonBonesObject;
	bool drawSkin = true;
	bool drawBones = true;
	bool playing = true;
	GMVec4 skeletonColor = GMVec4(0, 1, 0, 1);
	Map<GMModel*, GMSkeletalAnimationEvaluator*> modelEvaluatorMap;
};

class GMSkeletalGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMSkeletalGameObject, GMGameObject)
	GM_DECLARE_PROPERTY(SkeletonColor, skeletonColor)
	GM_DECLARE_PROPERTY(DrawSkin, drawSkin)

public:
	enum { AutoPlayFrame = -1 };
	using GMGameObject::GMGameObject;
	~GMSkeletalGameObject();

public:
	virtual void update(GMDuration dt) override;
	virtual void draw() override;

public:
	/*
	GMint32 getFramesCount();
	*/
	void createSkeletonBonesObject();
	void setDrawBones(bool b);
	void play();
	void pause();
	void reset(bool update);

public:
	inline void setFrame(GMint32 frame, GMint32 frameInterpolate) GM_NOEXCEPT
	{
		D(d);
		d->frame = frame;
		d->frameInterpolate = frameInterpolate;
	}

	inline bool getDrawBones() GM_NOEXCEPT
	{
		D(d);
		return d->drawBones;
	}

	inline bool isPlaying() GM_NOEXCEPT
	{
		D(d);
		return d->playing;
	}

private:
	void updateSkeleton();
	void initSkeletonBonesMesh(GMPart* mesh);
	void updateModel(GMModel* model, GMSkeletalAnimationEvaluator* evaluator);
};

END_NS
#endif