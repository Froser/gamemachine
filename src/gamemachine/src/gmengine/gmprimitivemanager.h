#ifndef __GMPRIMITIVEMANAGER_H__
#define __GMPRIMITIVEMANAGER_H__
#include <gmcommon.h>
BEGIN_NS

class GMSprite2DGameObject;
typedef GMlong GMPrimitiveID;

GM_INTERFACE(IPrimitive)
{
	virtual void drawPrimitive() = 0;
};

GM_PRIVATE_OBJECT(GMPrimitiveManager)
{
	const IRenderContext* context = nullptr;
	GMAtomic<GMPrimitiveID> current;
	Map<GMPrimitiveID, GMOwnedPtr<IPrimitive>> primitives;

// 图元
	GMOwnedPtr<GMSprite2DGameObject> line2D;
};

class GMPrimitiveManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMPrimitiveManager)

public:
	GMPrimitiveManager(const IRenderContext* context);

public:
	GMPrimitiveID addLine(const GMPoint& p1, const GMPoint& p2, const GMVec4& color);
	void render();

public:
	GMSprite2DGameObject* getLine2D() GM_NOEXCEPT
	{
		D(d);
		return d->line2D.get();
	}

private:
	GMPrimitiveID allocId();
};

END_NS
#endif