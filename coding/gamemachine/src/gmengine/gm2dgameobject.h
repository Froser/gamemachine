#ifndef __GM2DGAMEOBJECT_H__
#define __GM2DGAMEOBJECT_H__
#include <gmcommon.h>
#include "gmgameobject.h"
#include "gmassets.h"
#include <gmprimitivecreator.h>
BEGIN_NS

class GMGameWorld;
GM_PRIVATE_OBJECT(GM2DGameObject)
{
	GMRect geometry{ 0,0,0,0 };
};

class GM2DGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GM2DGameObject)

public:
	GM2DGameObject() = default;

public:
	void setGeometry(const GMRect& rect);

public:
	virtual bool canDeferredRendering() override { return false; }
};

//////////////////////////////////////////////////////////////////////////


//GMGlyphObject
struct GlyphProperties
{
};

GM_PRIVATE_OBJECT(GMGlyphObject)
{
	GMString lastRenderText;
	GMString text;
	GlyphProperties properties;
	bool autoResize = true; // 是否按照屏幕大小调整字体
	GMRectF lastGeometry = { -1 };
	GMRect lastClientRect = { -1 };
	ITexture* texture;
};

class GMGlyphObject : public GM2DGameObject
{
	DECLARE_PRIVATE(GMGlyphObject)

public:
	GMGlyphObject() = default;
	~GMGlyphObject();

public:
	void setText(const GMWchar* text);
	void update();

public:
	inline void setAutoResize(bool b) { D(d); d->autoResize = b; }

private:
	virtual void draw() override;
	virtual void onAppendingObjectToWorld() override;

private:
	void constructModel();
	void updateModel();
	void createVertices(GMComponent* component);
};

//////////////////////////////////////////////////////////////////////////
enum class GMImage2DAnchor
{
	Center,
};

GM_PRIVATE_OBJECT(GMImage2DGameObject)
{
	GMModel* model = nullptr;
	ITexture* image = nullptr;
	GMImage2DAnchor anchor = GMImage2DAnchor::Center;
};

class GMImage2DGameObject : public GM2DGameObject, public IPrimitiveCreatorShaderCallback
{
	DECLARE_PRIVATE(GMImage2DGameObject)

public:
	GMImage2DGameObject() = default;
	~GMImage2DGameObject();

public:
	void setImage(GMAsset& asset);

	//IPrimitiveCreatorShaderCallback
private:
	virtual void onCreateShader(Shader& shader) override;

private:
	virtual void onAppendingObjectToWorld();
};

END_NS
#endif