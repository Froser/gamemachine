#ifndef __HELPER_H__
#define __HELPER_H__
#include <gamemachine.h>
#include <gmanimation.h>
#include <gm2dgameobject.h>

using namespace gm;

struct Helper
{
	static GMRect getMiddleRectOfWindow(const GMRect& rc, IWindow* window);
};

GM_ALIGNED_16(class) FontColorAnimationKeyframe : public GMAnimationKeyframe
{
	GM_DECLARE_ALIGNED_ALLOCATOR()

public:
	FontColorAnimationKeyframe(GMTextGameObject* textObject, GMVec4 color, GMfloat timePoint);

public:
	virtual void reset(GMObject* object) override;
	virtual void beginFrame(GMObject* object, GMfloat timeStart) override;
	virtual void endFrame(GMObject* object) override;
	virtual void update(GMObject* object, GMfloat time) override;

private:
	GMTextGameObject* m_textObject;
	GMfloat m_timeStart;
	GMVec4 m_defaultColor;
	GMVec4 m_targetColor;
	Map<GMGameObject*, GMVec4, std::less<GMGameObject*>, AlignedAllocator<Pair<GMGameObject*, GMVec4>> > m_colorMap;
};

#endif