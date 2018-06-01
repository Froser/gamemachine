﻿#ifndef __GM2DGAMEOBJECT_H__
#define __GM2DGAMEOBJECT_H__
#include <gmcommon.h>
#include "gmassets.h"
#include <gmutilities.h>
#include <gmgameobject.h>

BEGIN_NS

struct ITypoEngine;

GM_PRIVATE_OBJECT(GM2DGameObjectBase)
{
	GMRect renderRc;
	bool dirty = true;
	GMRect geometry = { 0 };
};

class GM2DGameObjectBase : public GMGameObject
{
	DECLARE_PRIVATE_AND_BASE(GM2DGameObjectBase, GMGameObject)

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
	void setShader(GMShader& shader);

	inline const GMRect& getRenderRect()
	{
		D(d);
		return d->renderRc;
	}

	inline void markDirty()
	{
		D(d);
		d->dirty = true;
	}

	inline void cleanDirty()
	{
		D(d);
		d->dirty = false;
	}

	inline bool isDirty()
	{
		D(d);
		return d->dirty;
	}

protected:
	static GMRectF toViewportRect(const GMRect& rc, const GMRect& renderRc);
};

enum GMTextColorType
{
	Plain,
	ByScript,
};

GM_PRIVATE_OBJECT(GMTextGameObject)
{
	GMString text;
	GMint lineHeight = 0;
	bool center = false;
	GMsize_t length = 0;
	ITexture* texture = nullptr;
	ITypoEngine* typoEngine = nullptr;
	bool insetTypoEngine = true;
	GMModel* model = nullptr;
	GMTextColorType colorType = ByScript;
	GMFloat4 color = GMFloat4(1, 1, 1, 1);
	Vector<GMVertex> vericesCache;
};

class GMTextGameObject : public GM2DGameObjectBase
{
	DECLARE_PRIVATE_AND_BASE(GMTextGameObject, GM2DGameObjectBase)

public:
	GMTextGameObject(const GMRect& renderRc);
	GMTextGameObject(const GMRect& renderRc, ITypoEngine* typo);
	~GMTextGameObject();

public:
	void setText(const GMString& text);
	void setColorType(GMTextColorType type);
	void setColor(const GMVec4& color);
	void setCenter(bool center);

public:
	virtual void onAppendingObjectToWorld() override;
	virtual void draw() override;

private:
	void update();
	GMModel* createModel();
	void updateVertices(GMModel* model);
};

GM_PRIVATE_OBJECT(GMSprite2DGameObject)
{
	GMModel* model = nullptr;
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
	DECLARE_PRIVATE_AND_BASE(GMSprite2DGameObject, GM2DGameObjectBase)

	enum
	{
		VerticesCount = 4
	};

public:
	using Base::Base;
	~GMSprite2DGameObject();

public:
	virtual void draw() override;

public:
	void setDepth(GMint depth);
	void setTexture(ITexture* tex);
	void setTextureSize(GMint width, GMint height);
	void setTextureRect(const GMRect& rect);
	void setColor(const GMVec4& color);

private:
	void update();
	GMModel* createModel();
	void updateVertices(GMModel* model);
	void updateTexture(GMModel* model);
};

//////////////////////////////////////////////////////////////////////////
enum {
	BorderAreaCount = 9,
};

GM_PRIVATE_OBJECT(GMImage2DBorder)
{
	GMAsset texture;
	GMRect borderTextureGeometry;
	GMModel *models[BorderAreaCount] = { nullptr };
	GMGameObject *objects[BorderAreaCount] = { nullptr };
	GMfloat width;
	GMfloat height;
	GMfloat cornerWidth;
	GMfloat cornerHeight;
};

END_NS
#endif
