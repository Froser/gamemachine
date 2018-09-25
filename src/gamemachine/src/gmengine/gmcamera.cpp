#include "stdafx.h"
#include "gmcamera.h"
#include "gameobjects/gmspritegameobject.h"
#include "foundation/gamemachine.h"
#include <gmdxincludes.h>

void GMFrustum::setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f)
{
	D(d);
	d->type = GMFrustumType::Orthographic;
	d->left = left;
	d->right = right;
	d->bottom = bottom;
	d->top = top;
	d->n = n;
	d->f = f;

	d->projectionMatrix = Ortho(d->left, d->right, d->bottom, d->top, d->n, d->f);
	d->dirty = true;
}

void GMFrustum::setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
{
	D(d);
	d->type = GMFrustumType::Perspective;
	d->fovy = fovy;
	d->aspect = aspect;
	d->n = n;
	d->f = f;

	d->projectionMatrix = Perspective(d->fovy, d->aspect, d->n, d->f);
	d->dirty = true;
}

void GMFrustum::getPlanes(GMFrustumPlanes& planes)
{
	D(d);
	const GMMat4& projection = getProjectionMatrix();
	const GMMat4& view = getViewMatrix();
	GMMat4 clipMat = view * projection;

	auto& runningState = GM.getRunningStates();
	GMVec4 f, n, left, right, top, bottom;

	GetFrustumPlanesFromProjectionViewModelMatrix(
		runningState.farZ,
		runningState.nearZ,
		clipMat,
		f,
		n,
		right,
		left,
		top,
		bottom
	);

	planes.rightPlane.normal = GMVec3(right);
	planes.rightPlane.intercept = right.getW();

	planes.leftPlane.normal = GMVec3(left);
	planes.leftPlane.intercept = left.getW();

	planes.topPlane.normal = GMVec3(top);
	planes.topPlane.intercept = top.getW();

	planes.bottomPlane.normal = GMVec3(bottom);
	planes.bottomPlane.intercept = bottom.getW();

	planes.nearPlane.normal = GMVec3(n);
	planes.nearPlane.intercept = n.getW();

	planes.farPlane.normal = GMVec3(f);
	planes.farPlane.intercept = f.getW();
}

//is a bounding box in the Frustum?
bool GMFrustum::isBoundingBoxInside(const GMFrustumPlanes& frustumPlanes, const GMVec3(&vertices)[8])
{
	const GMPlane* planes[] =
	{
		&frustumPlanes.farPlane,
		&frustumPlanes.nearPlane,
		&frustumPlanes.topPlane,
		&frustumPlanes.bottomPlane,
		&frustumPlanes.leftPlane,
		&frustumPlanes.rightPlane
	};

	for (int i = 0; i < 6; ++i)
	{
		//if a point is not behind this plane, try next plane
		if (planes[i]->classifyPoint(vertices[0]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i]->classifyPoint(vertices[1]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i]->classifyPoint(vertices[2]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i]->classifyPoint(vertices[3]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i]->classifyPoint(vertices[4]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i]->classifyPoint(vertices[5]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i]->classifyPoint(vertices[6]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i]->classifyPoint(vertices[7]) != POINT_BEHIND_PLANE)
			continue;

		//All vertices of the box are behind this plane
		return false;
	}

	return true;
}

void GMFrustum::updateViewMatrix(const GMMat4& viewMatrix)
{
	D(d);
	d->viewMatrix = viewMatrix;
	d->inverseViewMatrix = Inverse(viewMatrix);
	d->dirty = true;
}

const GMMat4& GMFrustum::getProjectionMatrix() const
{
	D(d);
	return d->projectionMatrix;
}

const GMMat4& GMFrustum::getViewMatrix() const
{
	D(d);
	return d->viewMatrix;
}

const GMMat4& GMFrustum::getInverseViewMatrix() const
{
	D(d);
	return d->inverseViewMatrix;
}

//////////////////////////////////////////////////////////////////////////
GMCamera::GMCamera()
{
	D(d);
	d->frustum.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	d->lookAt.position = GMVec3(0);
	d->lookAt.lookAt = GMVec3(0, 0, 1);
}

void GMCamera::setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
{
	D(d);
	d->frustum.setPerspective(fovy, aspect, n, f);
}

void GMCamera::setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f)
{
	D(d);
	d->frustum.setOrtho(left, right, bottom, top, n, f);
}

void GMCamera::synchronize(GMSpriteGameObject* gameObject)
{
	D(d);
	auto& ps = gameObject->getPositionState();
	d->lookAt.lookAt = ps.lookAt;
	d->lookAt.position = ps.position;
}

void GMCamera::synchronizeLookAt()
{
	D(d);
	d->frustum.updateViewMatrix(::getViewMatrix(d->lookAt));
}

void GMCamera::lookAt(const GMCameraLookAt& lookAt)
{
	D(d);
	d->lookAt = lookAt;
	d->frustum.updateViewMatrix(::getViewMatrix(lookAt));
}

GMVec3 GMCamera::getRayToWorld(const GMRect& renderRect, GMint32 x, GMint32 y) const
{
	D(d);
	GMVec3 world = Unproject(
		GMVec3(x, y, 1),
		0,
		0,
		renderRect.width,
		renderRect.height,
		d->frustum.getProjectionMatrix(),
		d->frustum.getViewMatrix(),
		Identity<GMMat4>()
	);

	return world - d->lookAt.position;
}

void GMCamera::getPlanes(GMFrustumPlanes& planes)
{
	D(d);
	d->frustum.getPlanes(planes);
}

bool GMCamera::isBoundingBoxInside(const GMFrustumPlanes& planes, const GMVec3(&vertices)[8])
{
	return GMFrustum::isBoundingBoxInside(planes, vertices);
}