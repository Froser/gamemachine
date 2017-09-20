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
	GMRect rect{ 0,0,0,0 };
};

class GM2DGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GM2DGameObject)

public:
	GM2DGameObject() = default;

public:
	void setRect(const GMRect& rect);

public:
	virtual bool canDeferredRendering() override { return false; }
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