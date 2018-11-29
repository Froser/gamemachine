#include "stdafx.h"
#include "gmdx11light.h"
#include "gmdx11techniques.h"

void GMDx11Light::activateLight(GMuint32 index, ITechnique* technique)
{
	D(d);
	D_BASE(db, GMLight);
	if (!d->effect)
	{
		GMDx11Technique* dxTechnique = gm_cast<GMDx11Technique*>(technique);
		d->effect = dxTechnique->getEffect();
		GM_ASSERT(d->effect);
	}

#if GM_DEBUG
	GMDx11Technique* dxTechnique = gm_cast<GMDx11Technique*>(technique);
	GM_ASSERT(d->effect == dxTechnique->getEffect());
#endif

	if (!d->lightAttributes)
	{
		d->lightAttributes = d->effect->GetVariableByName("GM_LightAttributes");
		GM_ASSERT(d->lightAttributes->IsValid());
	}

	ID3DX11EffectVariable* lightStruct = d->lightAttributes->GetElement(index);
	GM_ASSERT(lightStruct->IsValid());

	ID3DX11EffectVectorVariable* position = lightStruct->GetMemberByName("Position")->AsVector();
	GM_ASSERT(position->IsValid());
	GM_DX_HR(position->SetFloatVector(db->position));

	ID3DX11EffectVectorVariable* color = lightStruct->GetMemberByName("Color")->AsVector();
	GM_ASSERT(color->IsValid());
	GM_DX_HR(color->SetFloatVector(db->color));

	ID3DX11EffectScalarVariable* type = lightStruct->GetMemberByName("Type")->AsScalar();
	GM_ASSERT(type->IsValid());
	GM_DX_HR(type->SetInt(getLightType()));

	ID3DX11EffectVariable* attenuation = lightStruct->GetMemberByName("Attenuation");
	GM_ASSERT(attenuation->IsValid());

	ID3DX11EffectScalarVariable* attenuationConstant = attenuation->GetMemberByName("Constant")->AsScalar();
	GM_ASSERT(attenuationConstant->IsValid());
	GM_DX_HR(attenuationConstant->SetFloat(db->attenuation.constant));

	ID3DX11EffectScalarVariable* attenuationLinear = attenuation->GetMemberByName("Linear")->AsScalar();
	GM_ASSERT(attenuationLinear->IsValid());
	GM_DX_HR(attenuationLinear->SetFloat(db->attenuation.linear));

	ID3DX11EffectScalarVariable* attenuationExp = attenuation->GetMemberByName("Exp")->AsScalar();
	GM_ASSERT(attenuationExp->IsValid());
	GM_DX_HR(attenuationExp->SetFloat(db->attenuation.exp));
}
