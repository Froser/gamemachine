#include "stdafx.h"
#include "gmcamera.h"
#include "gmspritegameobject.h"

GMCamera::GMCamera()
{
	D(d);
	d->frustum.initFrustum(75.f, 1.333f, .1f, 3200);
	d->state = { 0 };
}

void GMCamera::synchronize(GMSpriteGameObject* gameObject)
{
	D(d);
}