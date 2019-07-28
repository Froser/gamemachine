#include "stdafx.h"
#include "gmcamera.h"
#include "gameobjects/gmspritegameobject.h"
#include "foundation/gamemachine.h"
#include <gmdxincludes.h>

BEGIN_NS

inline GMMat4 getViewMatrixFromLookAt(const GMCameraLookAt& lookAt)
{
	return LookAt(lookAt.position, lookAt.lookDirection + lookAt.position, lookAt.up);
}

GM_PRIVATE_OBJECT_ALIGNED(GMFrustum)
{
	GMFrustumType type = GMFrustumType::Perspective;
	GMFrustumParameters parameters;
	GMfloat n;
	GMfloat f;

	GMMat4 projectionMatrix;
	GMMat4 viewMatrix;
	GMMat4 inverseViewMatrix;

	bool dirty = true;
};

bool GMFrustum::operator==(const GMFrustum& rhs)
{
	D(d);
	D_OF(d_rhs, &rhs);
	bool sameType = d->type == d_rhs->type;
	if (!sameType)
		return false;

	bool nf = getNear() == rhs.getNear() && getFar() == rhs.getFar();
	if (!nf)
		return false;

	if (d->type == GMFrustumType::Perspective)
		return d->parameters.fovy == d_rhs->parameters.fovy && d->parameters.aspect == d_rhs->parameters.aspect;
	
	return d->parameters.left == d_rhs->parameters.left &&
		d->parameters.right == d_rhs->parameters.right &&
		d->parameters.top == d_rhs->parameters.top &&
		d->parameters.bottom == d_rhs->parameters.bottom;
}

bool GMFrustum::operator!=(const GMFrustum& rhs)
{
	return !(*this == rhs);
}

GM_DEFINE_GETTER(GMFrustum, GMfloat, Near, n)
GM_DEFINE_GETTER(GMFrustum, GMfloat, Far, f)
GM_DEFINE_GETTER(GMFrustum, GMFrustumParameters, Parameters, parameters)

GMFrustum::GMFrustum()
{
	GM_CREATE_DATA();
}

GMFrustum::~GMFrustum()
{

}

GMFrustum::GMFrustum(const GMFrustum& rhs)
{
	*this = rhs;
}

GMFrustum::GMFrustum(GMFrustum&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMFrustum& GMFrustum::operator=(const GMFrustum& rhs)
{
	GM_COPY(rhs);
	return *this;
}

GMFrustum& GMFrustum::operator=(GMFrustum&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}

void GMFrustum::setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f)
{
	D(d);
	d->type = GMFrustumType::Orthographic;
	d->parameters.left = left;
	d->parameters.right = right;
	d->parameters.bottom = bottom;
	d->parameters.top = top;
	d->n = n;
	d->f = f;

	d->projectionMatrix = Ortho(d->parameters.left, d->parameters.right, d->parameters.bottom, d->parameters.top, d->n, d->f);
	d->dirty = true;
}

void GMFrustum::setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
{
	D(d);
	d->type = GMFrustumType::Perspective;
	d->parameters.fovy = fovy;
	d->parameters.aspect = aspect;
	d->n = n;
	d->f = f;

	d->projectionMatrix = Perspective(d->parameters.fovy, d->parameters.aspect, d->n, d->f);
	d->dirty = true;
}

void GMFrustum::getPlanes(GMFrustumPlanes& planes) const
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

	planes.rightPlane = right;
	planes.leftPlane = left;
	planes.topPlane = top;
	planes.bottomPlane = bottom;
	planes.nearPlane = n;
	planes.farPlane = f;
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
		if (planes[i]->classifyPoint(vertices[0]) != GMPointPosition::PointBehindPlane)
			continue;
		if (planes[i]->classifyPoint(vertices[1]) != GMPointPosition::PointBehindPlane)
			continue;
		if (planes[i]->classifyPoint(vertices[2]) != GMPointPosition::PointBehindPlane)
			continue;
		if (planes[i]->classifyPoint(vertices[3]) != GMPointPosition::PointBehindPlane)
			continue;
		if (planes[i]->classifyPoint(vertices[4]) != GMPointPosition::PointBehindPlane)
			continue;
		if (planes[i]->classifyPoint(vertices[5]) != GMPointPosition::PointBehindPlane)
			continue;
		if (planes[i]->classifyPoint(vertices[6]) != GMPointPosition::PointBehindPlane)
			continue;
		if (planes[i]->classifyPoint(vertices[7]) != GMPointPosition::PointBehindPlane)
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

void GMFrustum::cleanDirty()
{
	D(d);
	d->dirty = false;
}

bool GMFrustum::isDirty()
{
	D(d);
	return d->dirty;
}

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT_ALIGNED(GMCamera)
{
	GMFrustum frustum;
	GMCameraLookAt lookAt;
};

GM_DEFINE_GETTER(GMCamera, GMCameraLookAt, LookAt, lookAt)
GM_DEFINE_GETTER(GMCamera, GMFrustum, Frustum, frustum)

GMCamera::GMCamera()
{
	GM_CREATE_DATA();
	D(d);
	d->frustum.setPerspective(Radians(75.f), 1.333f, .1f, 3200);
	d->lookAt.position = GMVec3(0);
	d->lookAt.lookDirection = GMVec3(0, 0, 1);
}

GMCamera::GMCamera(const GMCamera& rhs)
{
	*this = rhs;
}

GMCamera::GMCamera(GMCamera&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMCamera& GMCamera::operator=(const GMCamera& rhs)
{
	GM_COPY(rhs);
	return *this;
}

GMCamera& GMCamera::operator=(GMCamera&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}

GMCamera::~GMCamera()
{

}

bool GMCamera::operator==(const GMCamera& rhs)
{
	D(d);
	D_OF(d_rhs, &rhs);
	return d->frustum == d_rhs->frustum && d->lookAt == d_rhs->lookAt;
}

bool GMCamera::operator!=(const GMCamera& rhs)
{
	return !(*this == rhs);
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

void GMCamera::updateViewMatrix()
{
	D(d);
	d->frustum.updateViewMatrix(getViewMatrixFromLookAt(d->lookAt));
}

void GMCamera::lookAt(const GMCameraLookAt& lookAt)
{
	D(d);
	d->lookAt = lookAt;
	d->frustum.updateViewMatrix(getViewMatrixFromLookAt(lookAt));
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

bool GMCamera::isDirty() const
{
	D(d);
	return d->frustum.isDirty();
}

void GMCamera::cleanDirty()
{
	D(d);
	d->frustum.cleanDirty();
}

const GMMat4& GMCamera::getProjectionMatrix() const
{
	D(d);
	return d->frustum.getProjectionMatrix();
}

const GMMat4& GMCamera::getViewMatrix() const
{
	D(d);
	return d->frustum.getViewMatrix();
}

const GMMat4& GMCamera::getInverseViewMatrix() const
{
	D(d);
	return d->frustum.getInverseViewMatrix();
}

bool GMCamera::isBoundingBoxInside(const GMFrustumPlanes& planes, const GMVec3(&vertices)[8])
{
	return GMFrustum::isBoundingBoxInside(planes, vertices);
}

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT_ALIGNED(GMCameraUtility)
{
	GMCamera* camera;
	GMfloat limitPitch = Radians(85.f);
	GMVec3 position;
	GMVec3 lookDirection;
};

GM_DEFINE_PROPERTY(GMCameraUtility, GMfloat, LimitPitch, limitPitch)
GMCameraUtility::GMCameraUtility(GMCamera* camera)
{
	GM_CREATE_DATA();
	setCamera(camera);
}

GMCameraUtility::~GMCameraUtility()
{

}

void GMCameraUtility::update(GMfloat yaw, GMfloat pitch)
{
	D(d);
	if (d->camera)
	{
		GMVec3 lookDirection = d->lookDirection;
		GMFloat4 f4_lookAt;
		lookDirection.loadFloat4(f4_lookAt);

		// 不考虑roll，把lookDirection投影到世界坐标系平面
		GMVec3 lookAt_z = GMVec3(f4_lookAt[0], 0, f4_lookAt[2]);
		// 计算pitch是否超出范围，不考虑roll
		GMfloat calculatedPitch = Asin(f4_lookAt[1]) + pitch;
		if (-d->limitPitch < calculatedPitch && calculatedPitch <= d->limitPitch)
		{
			// 找到lookAt_z垂直的一个向量，使用与世界坐标相同的坐标系
			GMVec3 lookAt_x = GMVec3(1, 0, 0) * GMQuat(GMVec3(0, 0, 1), lookAt_z);
			GMQuat qPitch = Rotate(-pitch, FastNormalize(lookAt_x));
			lookDirection = lookDirection * qPitch;
		}

		GMQuat qYaw = Rotate(-yaw, GMVec3(0, 1, 0));
		d->lookDirection = FastNormalize(lookDirection * qYaw);
		d->camera->lookAt(GMCameraLookAt(d->lookDirection, d->position));
	}
	else
	{
		gm_warning(gm_dbg_wrap("No camera in GMCameraUtility instance."));
	}
}

void GMCameraUtility::setCamera(GMCamera* camera)
{
	D(d);
	d->camera = camera;
	if (camera)
	{
		d->position = camera->getLookAt().position;
		d->lookDirection = camera->getLookAt().lookDirection;
	}
}

END_NS