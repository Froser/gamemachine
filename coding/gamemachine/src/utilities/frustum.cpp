#include "stdafx.h"
#include "Frustum.h"

enum FRUSTUM_PLANES
{
	LEFT_PLANE = 0,
	RIGHT_PLANE,
	TOP_PLANE,
	BOTTOM_PLANE,
	NEAR_PLANE,
	FAR_PLANE
};

Frustum::Frustum(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f)
	: m_fovy(fovy)
	, m_aspect(aspect)
	, m_n(n)
	, m_f(f)
{

}

void Frustum::update()
{
	vmath::mat4 projection = getPerspective();
	vmath::mat4& view = m_viewMatrix;
	vmath::mat4 clipMat;

	//Multiply the matrices
	clipMat = projection * view;

	GMfloat clip[16];
	for (GMint i = 0; i < 4; i++)
	{
		vmath::vec4 vec = clipMat[i];
		for (GMint j = 0; j < 4; j++)
		{
			clip[i * 4 + j] = vec[j];
		}
	}

	//calculate planes
	planes[RIGHT_PLANE].normal.setX(clip[3] - clip[0]);
	planes[RIGHT_PLANE].normal.setY(clip[7] - clip[4]);
	planes[RIGHT_PLANE].normal.setZ(clip[11] - clip[8]);
	planes[RIGHT_PLANE].intercept = clip[15] - clip[12];

	planes[LEFT_PLANE].normal.setX(clip[3] + clip[0]);
	planes[LEFT_PLANE].normal.setY(clip[7] + clip[4]);
	planes[LEFT_PLANE].normal.setZ(clip[11] + clip[8]);
	planes[LEFT_PLANE].intercept = clip[15] + clip[12];

	planes[BOTTOM_PLANE].normal.setX(clip[3] + clip[1]);
	planes[BOTTOM_PLANE].normal.setY(clip[7] + clip[5]);
	planes[BOTTOM_PLANE].normal.setZ(clip[11] + clip[9]);
	planes[BOTTOM_PLANE].intercept = clip[15] + clip[13];

	planes[TOP_PLANE].normal.setX(clip[3] - clip[1]);
	planes[TOP_PLANE].normal.setY(clip[7] - clip[5]);
	planes[TOP_PLANE].normal.setZ(clip[11] - clip[9]);
	planes[TOP_PLANE].intercept = clip[15] - clip[13];

	planes[FAR_PLANE].normal.setX(clip[3] - clip[2]);
	planes[FAR_PLANE].normal.setY(clip[7] - clip[6]);
	planes[FAR_PLANE].normal.setZ(clip[11] - clip[10]);
	planes[FAR_PLANE].intercept = clip[15] - clip[14];

	planes[NEAR_PLANE].normal.setX(clip[3] + clip[2]);
	planes[NEAR_PLANE].normal.setY(clip[7] + clip[6]);
	planes[NEAR_PLANE].normal.setZ(clip[11] + clip[10]);
	planes[NEAR_PLANE].intercept = clip[15] + clip[14];

	//normalize planes
	for (int i = 0; i < 6; ++i)
		planes[i].normalize();
}

//is a point in the Frustum?
bool Frustum::isPointInside(const btVector3 & point)
{
	for (int i = 0; i < 6; ++i)
	{
		if (planes[i].classifyPoint(point) == POINT_BEHIND_PLANE)
			return false;
	}

	return true;
}

//is a bounding box in the Frustum?
bool Frustum::isBoundingBoxInside(const btVector3 * vertices)
{
	//loop through planes
	for (int i = 0; i < 6; ++i)
	{
		//if a point is not behind this plane, try next plane
		if (planes[i].classifyPoint(vertices[0]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[1]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[2]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[3]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[4]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[5]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[6]) != POINT_BEHIND_PLANE)
			continue;
		if (planes[i].classifyPoint(vertices[7]) != POINT_BEHIND_PLANE)
			continue;

		//All vertices of the box are behind this plane
		return false;
	}

	return true;
}

vmath::mat4 Frustum::getPerspective()
{
	return vmath::mat4 (vmath::perspective(m_fovy, m_aspect, m_n, m_f));;
}

void Frustum::updateViewMatrix(vmath::mat4& viewMatrix, vmath::mat4& projMatrix)
{
	m_viewMatrix = viewMatrix;
	m_projMatrix = projMatrix;
}