#ifndef __GMSKELETONGAMEOBJECT_H__
#define __GMSKELETONGAMEOBJECT_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMSkeletonGameObject)
{
	enum { AutoPlayFrame = -1 };

	GMDuration animationTime = 0;
	GMDuration animationDuration = 0;
	GMDuration frameDuration = 0;
	GMint frame = AutoPlayFrame;
	GMint frameInterpolate = 0;
	GMOwnedPtr<GMGameObject> skeletonObject;
	bool drawSkin = true;
	bool drawSkeleton = true;
	GMVec4 skeletonColor = GMVec4(0, 1, 0, 1);
};

class GMSkeletonGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMSkeletonGameObject, GMGameObject)
	GM_DECLARE_PROPERTY(SkeletonColor, skeletonColor, GMVec4)
	GM_DECLARE_PROPERTY(DrawSkeleton, drawSkeleton, bool)
	GM_DECLARE_PROPERTY(DrawSkin, drawSkin, bool)

public:
	enum { AutoPlayFrame = -1 };
	using GMGameObject::GMGameObject;

public:
	virtual void update(GMDuration dt) override;
	virtual void draw() override;

public:
	GMint getFramesCount();
	void createSkeletonObject();

public:
	inline void setFrame(GMint frame, GMint frameInterpolate) GM_NOEXCEPT
	{
		D(d);
		d->frame = frame;
		d->frameInterpolate = frameInterpolate;
	}

private:
	void initAnimation();
	void getAdjacentTwoFrames(GMDuration dt, REF GMint& frame0, REF GMint& frame1, REF GMfloat& interpolate);
	void updateMesh(GMSkeletonMesh& mesh, const GMFrameSkeleton& frameSkeleton);
	void updateSkeleton();
	void initSkeletonMesh(GMMesh* mesh);
};

END_NS
#endif