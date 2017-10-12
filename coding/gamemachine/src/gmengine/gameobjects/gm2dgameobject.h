#ifndef __GM2DGAMEOBJECT_H__
#define __GM2DGAMEOBJECT_H__
#include <gmcommon.h>
#include "gmassets.h"
#include "gmcontrolgameobject.h"
#include <gmprimitivecreator.h>
#include <input.h>
BEGIN_NS

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

class GMGlyphObject : public GMControlGameObject
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

public:
	virtual void onAppendingObjectToWorld() override;
	virtual void draw() override;

private:
	void constructModel();
	void updateModel();
	void createVertices(GMComponent* component);

protected:
	virtual void updateUI() {} //Ignore base
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(GMImage2DGameObject)
{
	ITexture* image = nullptr;
	AUTORELEASE GMGlyphObject* textModel = nullptr;
	std::wstring text;
};

class GMImage2DGameObject : public GMControlGameObject, public IPrimitiveCreatorShaderCallback
{
	DECLARE_PRIVATE(GMImage2DGameObject)

	typedef GMControlGameObject Base;

public:
	GMImage2DGameObject() = default;
	~GMImage2DGameObject();

public:
	void setImage(GMAsset& asset);
	void setText(const GMString& text);

	//IPrimitiveCreatorShaderCallback
private:
	virtual void onCreateShader(Shader& shader) override;

private:
	virtual void onAppendingObjectToWorld();
	virtual void draw() override;
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(GMListbox2DGameObject)
{
	Vector<GMString> items;
	Vector<GMModel*> itemModels;
};

class GMListbox2DGameObject : public GMControlGameObject, public IPrimitiveCreatorShaderCallback
{
	DECLARE_PRIVATE(GMListbox2DGameObject);

public:
	GMListbox2DGameObject() = default;

private:
	virtual void onCreateShader(Shader& shader) override;
	virtual void onAppendingObjectToWorld();
};

END_NS
#endif