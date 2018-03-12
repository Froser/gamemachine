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

	d->mvpMatrix.projMatrix = Ortho(d->left, d->right, d->bottom, d->top, d->n, d->f);
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
}

void GMFrustum::getPlanes(GMFrustumPlanes& planes)
{
	D(d);
	const GMMat4& projection = getProjectionMatrix();
	const GMMat4& view = getViewMatrix();
	GMMat4 clipMat = view * projection;

	auto& runningState = GM.getGameMachineRunningStates();
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
	d->mvpMatrix.viewMatrix = viewMatrix;
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

const GMMat4& GMFrustum::getModelMatrix()
{
	D(d);
	return d->mvpMatrix.modelMatrix;
}

#if GM_USE_DX11
bool GMFrustum::createDxMatrixBuffer()
{
	D(d);
	// 定义统一MVP Matrix缓存
	D3D11_BUFFER_DESC vpBufferDesc;
	vpBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vpBufferDesc.ByteWidth = sizeof(GMMVPMatrix);
	vpBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vpBufferDesc.CPUAccessFlags = 0;
	vpBufferDesc.MiscFlags = 0;
	vpBufferDesc.StructureByteStride = 0;

	GMComPtr<ID3D11Device> device;
	bool suc = GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&device);
	GM_ASSERT(suc);
	HRESULT hr = device->CreateBuffer(&vpBufferDesc, NULL, &d->dxMatrixBuffer);
	GM_COM_CHECK_RETURN(hr, false);
	return S_OK == hr;
}

void GMFrustum::setDxModelMatrix(const GMMat4& matrix)
{
	D(d);
	d->mvpMatrix.modelMatrix = matrix;
	GM.getGraphicEngine()->update(GMUpdateDataType::ModelMatrix);
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