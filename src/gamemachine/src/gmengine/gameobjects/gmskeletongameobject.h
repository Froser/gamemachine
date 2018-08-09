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
};

class GMSkeletonGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMSkeletonGameObject, GMGameObject)

public:
	enum { AutoPlayFrame = -1 };
	using GMGameObject::GMGameObject;

public:
	virtual void update(GMDuration dt);

public:
	GMint getFramesCount();

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
};

END_NS
#endif