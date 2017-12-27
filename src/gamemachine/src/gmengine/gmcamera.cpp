#include "stdafx.h"
#include "gmcamera.h"
#include "gameobjects/gmspritegameobject.h"
#include "foundation/gamemachine.h"

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
	d->projMatrix = glm::ortho(d->left, d->right, d->bottom, d->top, d->n, d->f);
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
	d->projMatrix = glm::perspective(d->fovy, d->aspect, d->n, d->f);
	update();
}

void GMFrustum::update()
{
	D(d);
	glm::mat4 projection = getProjectionMatrix();
	glm::mat4& view = d->viewMatrix;
	glm::mat4 clipMat;

	if (d->type == GMFrustumType::Perspective)
	{
		//Multiply the matrices
		clipMat = projection * view;

		GMfloat clip[16];
		for (GMint i = 0; i < 4; i++)
		{
			glm::vec4 vec = clipMat[i];
			for (GMint j = 0; j < 4; j++)
			{
				clip[i * 4 + j] = vec[j];
			}
		}

		//calculate planes
		d->planes[RIGHT_PLANE].normal = glm::vec3(clip[3] - clip[0], clip[7] - clip[4], clip[11] - clip[8]);
		d->planes[RIGHT_PLANE].intercept = clip[15] - clip[12];

		d->planes[LEFT_PLANE].normal = glm::vec3(clip[3] + clip[0], clip[7] + clip[4], clip[11] + clip[8]);
		d->planes[LEFT_PLANE].intercept = clip[15] + clip[12];

		d->planes[BOTTOM_PLANE].normal = glm::vec3(clip[3] + clip[1], clip[7] + clip[5], clip[11] + clip[9]);
		d->planes[BOTTOM_PLANE].intercept = clip[15] + clip[13];

		d->planes[TOP_PLANE].normal = glm::vec3(clip[3] - clip[1], clip[7] - clip[5], clip[11] - clip[9]);
		d->planes[TOP_PLANE].intercept = clip[15] - clip[13];

		d->planes[FAR_PLANE].normal = glm::vec3(clip[3] - clip[2], clip[7] - clip[6], clip[11] - clip[10]);
		d->planes[FAR_PLANE].intercept = clip[15] - clip[14];

		d->planes[NEAR_PLANE].normal = glm::vec3(clip[3] + clip[2], clip[7] + clip[6], clip[11] + clip[10]);
		d->planes[NEAR_PLANE].intercept = clip[15] + clip[14];
	}
	else
	{
		GM_ASSERT(d->type == GMFrustumType::Orthographic);
		d->planes[RIGHT_PLANE].normal = glm::vec3(1, 0, 0);
		d->planes[RIGHT_PLANE].intercept = d->right;

		d->planes[LEFT_PLANE].normal = glm::vec3(-1, 0, 0);
		d->planes[LEFT_PLANE].intercept = d->left;

		d->planes[BOTTOM_PLANE].normal = glm::vec3(0, -1, 0);
		d->planes[BOTTOM_PLANE].intercept = d->bottom;

		d->planes[TOP_PLANE].normal = glm::vec3(0, 1, 0);
		d->planes[TOP_PLANE].intercept = d->top;

		d->planes[NEAR_PLANE].normal = glm::vec3(0, 0, 1);
		d->planes[NEAR_PLANE].intercept = d->n;

		d->planes[FAR_PLANE].normal = glm::vec3(0, 0, -1);
		d->planes[FAR_PLANE].intercept = d->f;
	}

	//normalize planes
	for (int i = 0; i < 6; ++i)
		d->planes[i].normalize();
}

//is a point in the Frustum?
bool GMFrustum::isPointInside(const glm::vec3 & point)
{
	D(d);
	for (int i = 0; i < 6; ++i)
	{
		if (d->planes[i].classifyPoint(point) == POINT_BEHIND_PLANE)
			return false;
	}

	return true;
}

//is a bounding box in the Frustum?
bool GMFrustum::isBoundingBoxInside(const glm::vec3 * vertices)
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

void GMFrustum::updateViewMatrix(glm::mat4& viewMatrix)
{
	D(d);
	d->viewMatrix = viewMatrix;
	update();
}

//////////////////////////////////////////////////////////////////////////
GMCamera::GMCamera()
{
	D(d);
	d->frustum.setPerspective(glm::radians(75.f), 1.333f, .1f, 3200);
	d->state.position = glm::vec3(0);
	d->state.lookAt = glm::vec3(0, 0, 1);
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
	d->state = gameObject->getPositionState();
	d->lookAt.lookAt = d->state.lookAt;
	d->lookAt.position = d->state.position;
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