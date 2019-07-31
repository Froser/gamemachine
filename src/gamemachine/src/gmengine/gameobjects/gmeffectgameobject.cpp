#include "stdafx.h"
#include "gmeffectgameobject.h"

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMEffectGameObject)
{
	GMRenderTechniqueID techId = 0;
};

GMEffectGameObject::GMEffectGameObject(GMRenderTechniqueID techId, GMAsset asset)
	: GMGameObject(asset)
{
	GM_CREATE_DATA();
	D(d);
	d->techId = techId;
}

GMEffectGameObject::~GMEffectGameObject()
{

}

void GMEffectGameObject::onAppendingObjectToWorld()
{
	foreachModel([this](GMModel* model) {
		D(d);
		model->setTechniqueId(d->techId);
		model->setType(GMModelType::Custom);
	});
	Base::onAppendingObjectToWorld();
}

END_NS
