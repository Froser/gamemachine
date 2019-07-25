#ifndef __GM2DGAMEOBJECT_H__
#define __GM2DGAMEOBJECT_H__
#include <gmcommon.h>
#include "gmassets.h"
#include <gmutilities.h>
#include <gmgameobject.h>

BEGIN_NS

struct ITypoEngine;
class GMTypoTextBuffer;

GM_PRIVATE_CLASS(GM2DGameObjectBase);
class GM_EXPORT GM2DGameObjectBase : public GMGameObject
{
	GM_DECLARE_PRIVATE(GM2DGameObjectBase)
	GM_DECLARE_BASE(GMGameObject)

public:
	GM2DGameObjectBase(const GMRect& renderRc);

public:
	virtual bool canDeferredRendering();

public:
	void setGeometry(const GMRect& geometry);
	GMShader& getShader();

public:
	const GMRect& getGeometry();

protected:
	virtual void initShader(GMShader& shader);

protected:
	inline const GMRect& getRenderRect() GM_NOEXCEPT;
	inline void markDirty() GM_NOEXCEPT;
	inline void cleanDirty() GM_NOEXCEPT;
	inline bool isDirty() GM_NOEXCEPT;

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

GM_PRIVATE_CLASS(GMTextGameObject);
class GM_EXPORT GMTextGameObject : public GM2DGameObjectBase
{
	GM_DECLARE_PRIVATE(GMTextGameObject)
	GM_DECLARE_BASE(GM2DGameObjectBase);

public:
	GMTextGameObject(const GMRect& renderRc);
	GMTextGameObject(const GMRect& renderRc, ITypoEngine* typo);
	~GMTextGameObject();

public:
	void setText(const GMString& text);
	void setColorType(GMTextColorType type) GM_NOEXCEPT;
	void setColor(const GMVec4& color);
	GMVec4 getColor();
	void setCenter(bool center) GM_NOEXCEPT;
	void setFont(GMFontHandle font) GM_NOEXCEPT;
	void setNewline(bool newline) GM_NOEXCEPT;
	void setLineSpacing(GMint32 lineSpacing) GM_NOEXCEPT;
	void setFontSize(GMFontSizePt fontSize) GM_NOEXCEPT;

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
	ITypoEngine* getTypoEngine() GM_NOEXCEPT;

public:
	virtual GMModel* getModel() override;
	virtual void onAppendingObjectToWorld() override;
	virtual void draw() override;

private:
	void update();
	GMScene* createScene();
	GMModel* createModel();
	void updateVertices(GMScene* scene);
};

GM_PRIVATE_CLASS(GMSprite2DGameObject);
class GM_EXPORT GMSprite2DGameObject : public GM2DGameObjectBase
{
	GM_DECLARE_PRIVATE(GMSprite2DGameObject)
	GM_DECLARE_BASE(GM2DGameObjectBase)

	enum
	{
		VerticesCount = 4
	};

public:
	GMSprite2DGameObject(const GMRect& renderRc);
	~GMSprite2DGameObject();

public:
	virtual void draw() override;
	virtual void setColor(const GMVec4& color);

public:
	void setDepth(GMfloat depth);
	void setTexture(GMAsset tex);
	void setTextureSize(GMint32 width, GMint32 height);
	void setTextureRect(const GMRect& rect);

private:
	void update();

protected:
	virtual void updateVertices(GMScene* scene);
	virtual void updateTexture(GMScene* scene);
	virtual GMScene* createScene();
};

GM_PRIVATE_CLASS(GMBorder2DGameObject);
class GMBorder2DGameObject : public GMSprite2DGameObject
{
	GM_DECLARE_PRIVATE(GMBorder2DGameObject)
	GM_DECLARE_BASE(GMSprite2DGameObject)
	GM_FRIEND_CLASS(GMImage2DGameObject)

	enum
	{
		VerticesCount = 16
	};

public:
	GMBorder2DGameObject(const GMRect& renderRc);
	~GMBorder2DGameObject();

public:
	void setCornerRect(const GMRect& rc);

protected:
	virtual GMScene* createScene() override;
	virtual void updateVertices(GMScene* scene) override;
};

END_NS
#endif
