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
	void onCreateShader(Shader& shader);

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
	using GMControlGameObject::GMControlGameObject;
	~GMImage2DGameObject();

public:
	void setImage(GMAsset& asset);
	void setText(const GMString& text);

public:
	virtual void onAppendingObjectToWorld();
	virtual void draw() override;

	//IPrimitiveCreatorShaderCallback
private:
	virtual void onCreateShader(Shader& shader) override;
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(GMListbox2DGameObject)
{
	GMfloat margins[4] = { 0 };
};

class GMListbox2DGameObject : public GMImage2DGameObject
{
	DECLARE_PRIVATE(GMListbox2DGameObject);

	typedef GMImage2DGameObject Base;

	enum Margins
	{
		Left = 0,
		Top,
		Right,
		Bottom,
	};

public:
	GMImage2DGameObject* addItem(const GMString& text);
	void setItemMargins(GMfloat left, GMfloat top, GMfloat right, GMfloat bottom);

public:
	virtual void onAppendingObjectToWorld();

private:
	virtual void onCreateShader(Shader& shader) override;
	virtual void draw() override;
	virtual void notifyControl() override;

private:
	inline const Vector<GMControlGameObject*>& getItems() { D_BASE(db, GMControlGameObject); return db->children; }
};

END_NS
#endif