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

GM_PRIVATE_CLASS(GMPrimitive3DObject);
class GMPrimitive3DObject : public GMGameObject
{
	GM_DECLARE_PRIVATE(GMPrimitive3DObject)
	GM_DECLARE_BASE(GMGameObject)

public:
	GMPrimitive3DObject();
	GMPrimitive3DObject(GMSceneAsset asset);
	~GMPrimitive3DObject();

public:
	virtual void draw() override;

public:
	void setPosition(const GMVec3& p1, const GMVec3& p2);
	void setColor(const GMVec4& color);
	void markDirty() GM_NOEXCEPT;
	void cleanDirty() GM_NOEXCEPT;
	bool isDirty() GM_NOEXCEPT;

protected:
	virtual void update();
	virtual void initShader(GMShader& shader);
	virtual void updateVertices(GMModel* model) = 0;
	virtual GMModel* createModel() = 0;
};

GM_PRIVATE_CLASS(GMPrimitiveManager);
class GM_EXPORT GMPrimitiveManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMPrimitiveManager)

public:
	GMPrimitiveManager(const IRenderContext* context);
	~GMPrimitiveManager();

public:
	GMPrimitiveID addLine(const GMPoint& p1, const GMPoint& p2, const GMVec4& color);
	GMPrimitiveID addLine(const GMVec3& p1, const GMVec3& p2, const GMVec4& color);
	void remove(GMPrimitiveID);
	void clear();
	void render();
	GMSprite2DGameObject* getLine2D() GM_NOEXCEPT;
	GMPrimitive3DObject* getLine3D() GM_NOEXCEPT;

private:
	GMPrimitiveID allocId();
};

END_NS
#endif