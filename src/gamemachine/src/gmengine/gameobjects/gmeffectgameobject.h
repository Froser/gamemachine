#ifndef __GMEFFECTGAMEOBJECT_H__
#define __GMEFFECTGAMEOBJECT_H__
#include <gmcommon.h>
#include <gmgameobject.h>
BEGIN_NS

//! 读取一个特效模型的类。
GM_PRIVATE_CLASS(GMEffectGameObject);
class GMEffectGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE(GMEffectGameObject);
	GM_DECLARE_BASE(GMGameObject);

public:
	GMEffectGameObject(GMRenderTechniqueID, GMAsset);
	~GMEffectGameObject();

public:
	void setUniform(const GMString& name, GMVariant value);
	GMVariant getUniform(const GMString& name);

protected:
	virtual void onAppendingObjectToWorld() override;

public:
	virtual void onRenderShader(GMModel*, IShaderProgram* shaderProgram) const override;
};

END_NS
#endif