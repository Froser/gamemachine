#ifndef __GM2DGAMEOBJECT_H__
#define __GM2DGAMEOBJECT_H__
#include <gmcommon.h>
#include "gmassets.h"
#include <gmutilities.h>
#include <gmgameobject.h>

BEGIN_NS

struct ITypoEngine;
class GMTypoTextBuffer;

GM_PRIVATE_OBJECT(GM2DGameObjectBase)
{
	GMRect renderRc;
	bool dirty = true;
	GMRect geometry = { 0 };
};

class GM2DGameObjectBase : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GM2DGameObjectBase, GMGameObject)

public:
	GM2DGameObjectBase(const GMRect& renderRc);

public:
	void setGeometry(const GMRect& geometry);

public:
	inline const GMRect& getGeometry()
	{
		D(d);
		return d->geometry;
	}

protected:
	virtual void setShader(GMShader& shader);

	inline const GMRect& getRenderRect() GM_NOEXCEPT
	{
		D(d);
		return d->renderRc;
	}

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

protected:
	static GMRectF toViewportRect(const GMRect& rc, const GMRect& renderRc);
};

enum class GMTextColorType
{
	Plain,
	ByScript,
};

enum class GMTextDrawMode
{
	Immediate,
	UseBuffer,
};

GM_PRIVATE_OBJECT(GMTextGameObject)
{
	GMString text;
	GMint lineSpacing = 0;
	bool center = false;
	bool newline = true;
	GMsize_t length = 0;
	ITexture* texture = nullptr;
	ITypoEngine* typoEngine = nullptr;
	bool insetTypoEngine = true;
	GMFontHandle font = 0;
	GMModel* model = nullptr;
	GMAsset modelAsset;
	GMTextColorType colorType = GMTextColorType::ByScript;
	GMFloat4 color = GMFloat4(1, 1, 1, 1);
	Vector<GMVertex> vericesCache;
	GMTypoTextBuffer* textBuffer = nullptr;
	GMTextDrawMode drawMode = GMTextDrawMode::Immediate;
};

class GMTextGameObject : public GM2DGameObjectBase
{
	GM_DECLARE_PRIVATE_AND_BASE(GMTextGameObject, GM2DGameObjectBase)

public:
	GMTextGameObject(const GMRect& renderRc);
	GMTextGameObject(const GMRect& renderRc, ITypoEngine* typo);
	~GMTextGameObject();

public:
	void setText(const GMString& text);
	void setColorType(GMTextColorType type) GM_NOEXCEPT;
	void setColor(const GMVec4& color);
	void setCenter(bool center) GM_NOEXCEPT;
	void setFont(GMFontHandle font) GM_NOEXCEPT;
	void setNewline(bool newline) GM_NOEXCEPT;
	void setLineSpacing(GMint lineSpacing) GM_NOEXCEPT;

	//! 设置文本缓存。
	/*!
	  文本缓存是指一段已经排版好的文本。当此方法被调用时，setDrawMode被设置成GMTextDrawMode::UseBuffer，并且setText被设置成文本缓存的内容。<BR>
	  如果不想使用文本缓存，而是每次解析setText()传入的文本来渲染排版，请将setDrawMode设置为GMTextDrawMode::Immediate。
	  \param textBuffer 传入的文本缓存。
	*/
	void setTextBuffer(GMTypoTextBuffer* textBuffer) GM_NOEXCEPT;

	//! 设置渲染方式。
	/*!
	  指定文本的渲染方式。
	  \param mode 渲染方式。如果是GMTextDrawMode::Immediate，引擎每次都会解析调用者通过setText()传入的文本。
	  如果是GMTextDrawMode::UseBuffer，调用者需要通过setTextBuffer()传入一个GMTypoTextBuffer。绘制时，引擎不会重新解析文本，而是使用GMTypoTextBuffer已经解析好的内容。
	  \sa setTextBuffer()
	*/
	void setDrawMode(GMTextDrawMode mode) GM_NOEXCEPT;

public:
	virtual void onAppendingObjectToWorld() override;
	virtual void draw() override;

public:
	inline ITypoEngine* getTypoEngine() GM_NOEXCEPT
	{
		D(d);
		return d->typoEngine;
	}

private:
	void update();
	GMModel* createModel();
	void updateVertices(GMModel* model);
};

GM_PRIVATE_OBJECT(GMSprite2DGameObject)
{
	GMModel* model = nullptr;
	GMAsset modelAsset;
	ITexture* texture = nullptr;
	GMRect textureRc;
	GMint texHeight = 0;
	GMint texWidth = 0;
	GMfloat depth = 0;
	bool needUpdateTexture = true;
	GMFloat4 color = GMFloat4(1, 1, 1, 1);
};

class GMSprite2DGameObject : public GM2DGameObjectBase
{
	GM_DECLARE_PRIVATE_AND_BASE(GMSprite2DGameObject, GM2DGameObjectBase)

	enum
	{
		VerticesCount = 4
	};

public:
	using Base::Base;
	~GMSprite2DGameObject() = default;

public:
	virtual void draw() override;
	virtual void setColor(const GMVec4& color);

public:
	void setDepth(GMfloat depth);
	void setTexture(ITexture* tex);
	void setTextureSize(GMint width, GMint height);
	void setTextureRect(const GMRect& rect);

private:
	void update();

protected:
	virtual void updateVertices(GMModel* model);
	virtual void updateTexture(GMModel* model);
	virtual GMModel* createModel();
};

GM_PRIVATE_OBJECT(GMBorder2DGameObject)
{
	GMModel* model = nullptr;
	GMAsset modelAsset;
	GMRect corner;
};

class GMBorder2DGameObject : public GMSprite2DGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMBorder2DGameObject, GMSprite2DGameObject)
	GM_FRIEND_CLASS(GMImage2DGameObject)

	enum
	{
		VerticesCount = 16
	};

public:
	using GMSprite2DGameObject::GMSprite2DGameObject;
	~GMBorder2DGameObject() = default;

public:
	void setCornerRect(const GMRect& rc);

protected:
	virtual GMModel* createModel() override;
	virtual void updateVertices(GMModel* model) override;
};

END_NS
#endif
