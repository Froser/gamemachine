#ifndef __GMPRIMITIVEMANAGER_H__
#define __GMPRIMITIVEMANAGER_H__
#include <gmcommon.h>
#include <gmgameobject.h>
#include <gm2dgameobject.h>
BEGIN_NS

typedef GMlong GMPrimitiveID;

GM_INTERFACE(IPrimitive)
{
	virtual void drawPrimitive() = 0;
};

GM_PRIVATE_OBJECT(GMPrimitive3DObject)
{
	bool dirty = true;
	GMVec3 p1;
	GMVec3 p2;
	GMVec4 color;
	GMModelAsset modelAsset;
	GMModel* model = nullptr;
};

class GMPrimitive3DObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMPrimitive3DObject, GMGameObject)

public:
	using Base::Base;

public:
	virtual void draw() override;

public:
	void setPosition(const GMVec3& p1, const GMVec3& p2);
	void setColor(const GMVec4& color);

protected:
	virtual void update();
	virtual void initShader(GMShader& shader);
	virtual void updateVertices(GMModel* model) = 0;
	virtual GMModel* createModel() = 0;

public:
	inline void markDirty() GM_NOEXCEPT
	{
		D(d);
		d->dirty = true;
	}

	inline void cleanDirty() GM_NOEXCEPT
	{
		D(d);
		d->dirty = false;
	}

	inline bool isDirty() GM_NOEXCEPT
	{
		D(d);
		return d->dirty;
	}
};

GM_PRIVATE_OBJECT(GMPrimitiveManager)
{
	const IRenderContext* context = nullptr;
	GMAtomic<GMPrimitiveID> current;
	Map<GMPrimitiveID, GMOwnedPtr<IPrimitive>> primitives;

// 图元
	GMOwnedPtr<GMSprite2DGameObject> line2D;
	GMOwnedPtr<GMPrimitive3DObject> line3D;
};

class GMPrimitiveManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMPrimitiveManager)

public:
	GMPrimitiveManager(const IRenderContext* context);

public:
	GMPrimitiveID addLine(const GMPoint& p1, const GMPoint& p2, const GMVec4& color);
	GMPrimitiveID addLine(const GMVec3& p1, const GMVec3& p2, const GMVec4& color);
	void remove(GMPrimitiveID);
	void clear();
	void render();

public:
	GMSprite2DGameObject* getLine2D() GM_NOEXCEPT
	{
		D(d);
		return d->line2D.get();
	}

	GMPrimitive3DObject* getLine3D() GM_NOEXCEPT
	{
		D(d);
		return d->line3D.get();
	}

private:
	GMPrimitiveID allocId();
};

END_NS
#endif