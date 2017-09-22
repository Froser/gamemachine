#include "stdafx.h"
#include "gmcamera.h"
#include "gameobjects/gmspritegameobject.h"
#include "foundation/gamemachine.h"

GMCamera::GMCamera()
{
	D(d);
	d->frustum.initPerspective(75.f, 1.333f, .1f, 3200);
	d->state = { 0 };
}

void GMCamera::initPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
{
	D(d);
	d->frustum.initPerspective(fovy, aspect, n, f);
}

void GMCamera::initOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f)
{
	D(d);
	d->frustum.initOrtho(left, right, bottom, top, n, f);
}

void GMCamera::synchronize(GMSpriteGameObject* gameObject)
{
	D(d);
	d->state = gameObject->getPositionState();
	d->lookAt.lookAt[1] = sin(d->state.pitch);
	GMfloat l = cos(d->state.pitch);
	d->lookAt.lookAt[0] = l * sin(d->state.yaw);
	d->lookAt.lookAt[2] = -l * cos(d->state.yaw);
	d->lookAt.position = d->state.position;
}

void GMCamera::apply()
{
	D(d);
	GameMachine::instance().getGraphicEngine()->updateCameraView(d->lookAt);
}