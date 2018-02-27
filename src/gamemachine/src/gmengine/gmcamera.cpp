#include "stdafx.h"
#include "gmcamera.h"
#include "gameobjects/gmspritegameobject.h"
#include "foundation/gamemachine.h"
#include <gmdxincludes.h>

//Frustum
enum
{
	LEFT_PLANE = 0,
	RIGHT_PLANE,
	TOP_PLANE,
	BOTTOM_PLANE,
	NEAR_PLANE,
	FAR_PLANE
};

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

	d->mvpMatrix.projMatrix = Ortho(d->left, d->right, d->bottom, d->top, d->n, d->f);
	update();
}

void GMFrustum::setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
{
	D(d);
	d->type = GMFrustumType::Perspective;
	d->fovy = fovy;
	d->aspect = aspect;
	d->n = n;
	d->f = f;

	d->mvpMatrix.projMatrix = Perspective(d->fovy, d->aspect, d->n, d->f);
	update();
}

void GMFrustum::update()
{
	D(d);
	const GMMat4& projection = getProjectionMatrix();
	GMMat4& view = d->mvpMatrix.viewMatrix;
	GMMat4 clipMat;

	//Multiply the matrices
	clipMat = view * projection;

	GMVec4 f, n, left, right, top, bottom;
	//TODO 按照OpenGL坐标系，左上角为(-1, 1)，右下角为(1, -1), Z范围(-1, 1)
	//使用DirectX时，应该更改此处
	GetFrustumPlanesFromProjectionViewModelMatrix(
		1,
		-1,
		clipMat,
		f,
		n,
		right,
		left,
		top,
		bottom
	);

	d->planes[RIGHT_PLANE].normal = GMVec3(right);
	d->planes[RIGHT_PLANE].intercept = right.getW();

	d->planes[LEFT_PLANE].normal = GMVec3(left);
	d->planes[LEFT_PLANE].intercept = left.getW();

	d->planes[TOP_PLANE].normal = GMVec3(top);
	d->planes[TOP_PLANE].intercept = top.getW();

	d->planes[BOTTOM_PLANE].normal = GMVec3(bottom);
	d->planes[BOTTOM_PLANE].intercept = bottom.getW();

	d->planes[NEAR_PLANE].normal = GMVec3(n);
	d->planes[NEAR_PLANE].intercept = n.getW();

	d->planes[FAR_PLANE].normal = GMVec3(f);
	d->planes[FAR_PLANE].intercept = f.getW();
}

//is a bounding box in the Frustum?
bool GMFrustum::isBoundingBoxInside(const GMVec3(&vertices)[8])
{
	D(d);
	for (int i = 0; i < 6; ++i)
	{
		//if a point is not behind this plane, try next plane
		if (d->planes[i].classifyPoint(vertices[0]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[1]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[2]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[3]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[4]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[5]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[6]) != POINT_BEHIND_PLANE)
			continue;
		if (d->planes[i].classifyPoint(vertices[7]) != POINT_BEHIND_PLANE)
			continue;

		//All vertices of the box are behind this plane
		return false;
	}

	return true;
}

void GMFrustum::updateViewMatrix(const GMMat4& viewMatrix)
{
	D(d);
	d->mvpMatrix.viewMatrix = viewMatrix;
	update();
}

const GMMat4& GMFrustum::getProjectionMatrix()
{
	D(d);
	return d->mvpMatrix.projMatrix;
}

const GMMat4& GMFrustum::getViewMatrix()
{
	D(d);
	return d->mvpMatrix.viewMatrix;
}

#if GM_USE_DX11
void GMFrustum::setDxMatrixBuffer(GMComPtr<ID3D11Buffer> buffer)
{
	D(d);
	d->dxMatrixBuffer = buffer;
}

GMComPtr<ID3D11Buffer> GMFrustum::getDxMatrixBuffer()
{
	D(d);
	return d->dxMatrixBuffer;
}
#endif

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
	GM.getGraphicEngine()->update(GMUpdateDataType::ProjectionMatrix);
}

void GMCamera::setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f)
{
	D(d);
	d->frustum.setOrtho(left, right, bottom, top, n, f);
	GM.getGraphicEngine()->update(GMUpdateDataType::ProjectionMatrix);
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
	getFrustum().updateViewMatrix(::getViewMatrix(d->lookAt));
	GM.getGraphicEngine()->update(GMUpdateDataType::ViewMatrix);
}

void GMCamera::lookAt(const GMCameraLookAt& lookAt)
{
	D(d);
	d->lookAt = lookAt;
	getFrustum().updateViewMatrix(::getViewMatrix(lookAt));
	GM.getGraphicEngine()->update(GMUpdateDataType::ViewMatrix);
}

GMVec3 GMCamera::getRayToWorld(GMint x, GMint y) const
{
	D(d);
	const GMRect& cr = GM.getGameMachineRunningStates().clientRect;
	GMVec3 world = Unproject(
		GMVec3(x, y, 1),
		0,
		0,
		cr.width,
		cr.height,
		d->frustum.getProjectionMatrix(),
		d->frustum.getViewMatrix(),
		Identity<GMMat4>()
	);

	return world - d->lookAt.position;
}