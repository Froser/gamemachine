#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__
#include "common.h"
#include "plane.h"
#include "LinearMath\btVector3.h"
#include "vmath.h"
BEGIN_NS

class Frustum
{
public:
	Frustum(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);

public:
	void update();
	bool isPointInside(const btVector3& point);
	bool isBoundingBoxInside(const btVector3* vertices);
	vmath::mat4 getPerspective();
	void updateViewMatrix(vmath::mat4& viewMatrix, vmath::mat4& projMatrix);

private:
	Plane planes[6];
	GMfloat m_fovy;
	GMfloat m_aspect;
	GMfloat m_n;
	GMfloat m_f;
	vmath::mat4 m_viewMatrix;
	vmath::mat4 m_projMatrix;
};

END_NS
#endif