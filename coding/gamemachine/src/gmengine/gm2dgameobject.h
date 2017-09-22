#ifndef __GM2DGAMEOBJECT_H__
#define __GM2DGAMEOBJECT_H__
#include <gmcommon.h>
#include "gmgameobject.h"
#include "gmassets.h"
#include <gmprimitivecreator.h>
#include <gminput.h>
BEGIN_NS

enum class GM2DEventType
{
	MouseMove,
};

// 2D对象的事件
class GM2DEvent
{
public:
	GM2DEvent(GM2DEventType type);
	GM2DEventType type() { return m_type; }

private:
	GM2DEventType m_type;
};

class GM2DMouseMoveEvent : public GM2DEvent
{
public:
	GM2DMouseMoveEvent(const GMMouseState& ms)
		: GM2DEvent(GM2DEventType::MouseMove)
		, m_state(ms)
	{
	}
	GMMouseState state() { return m_state; }

private:
	GMMouseState m_state;
};

//////////////////////////////////////////////////////////////////////////
class GMGameWorld;
GM_PRIVATE_OBJECT(GM2DGameObject)
{
	GMRect geometry{ 0 };
	GMRect clientSize{ 0 };

	bool stretch = true;
};

class GM2DGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GM2DGameObject)

	GM_DECLARE_PROPERTY(Stretch, stretch, bool);

public:
	GM2DGameObject();

public:
	void setGeometry(const GMRect& rect);

public:
	virtual GMGameObjectType getType() { return GMGameObjectType::Controls; }
	virtual bool canDeferredRendering() override { return false; }
	virtual void notifyControl() override;

protected:
	virtual void event(GM2DEvent* e);
	virtual bool insideGeometry(GMint x, GMint y);
	virtual void updateUI();
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

protected:
	virtual void updateUI() {} //Ignore base
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