#include "stdafx.h"
#include "gmdx11light.h"
#include "gmdx11renderers.h"

void GMDx11Light::setLightPosition(GMfloat position[3])
{
	D(d);
	d->position[0] = position[0];
	d->position[1] = position[1];
	d->position[2] = position[2];
	d->position[3] = 1.0f;
}

void GMDx11Light::setLightColor(GMfloat color[3])
{
	D(d);
	d->color[0] = color[0];
	d->color[1] = color[1];
	d->color[2] = color[2];
	d->color[3] = 1.0f;
}

const GMfloat* GMDx11Light::getLightPosition() const
{
	D(d);
	return d->position;
}

const GMfloat* GMDx11Light::getLightColor() const
{
	D(d);
	return d->color;
}

void GMDx11Light::activateLight(GMuint index, IRenderer* renderer)
{
	static ID3DX11Effect* s_effect = nullptr;
	if (!s_effect)
	{
		GMDx11Renderer* dxRenderer = gm_cast<GMDx11Renderer*>(renderer);
		s_effect = dxRenderer->getEffect();
		GM_ASSERT(s_effect);
	}

#if _DEBUG
	GMDx11Renderer* dxRenderer = gm_cast<GMDx11Renderer*>(renderer);
	GM_ASSERT(s_effect == dxRenderer->getEffect());
#endif

	static ID3DX11EffectVariable* lightAttributes = s_effect->GetVariableByName("LightAttributes");
	GM_ASSERT(lightAttributes->IsValid());

	ID3DX11EffectVariable* lightStruct = lightAttributes->GetElement(index);
	GM_ASSERT(lightStruct->IsValid());

	ID3DX11EffectVectorVariable* position = lightStruct->GetMemberByName("Position")->AsVector();
	GM_ASSERT(position->IsValid());
	GM_DX_HR(position->SetFloatVector(getLightPosition()));

	ID3DX11EffectVectorVariable* color = lightStruct->GetMemberByName("Color")->AsVector();
	GM_ASSERT(color->IsValid());
	GM_DX_HR(color->SetFloatVector(getLightColor()));

	ID3DX11EffectScalarVariable* type = lightStruct->GetMemberByName("Type")->AsScalar();
	GM_ASSERT(type->IsValid());
	GM_DX_HR(type->SetInt(getLightType()));
}
