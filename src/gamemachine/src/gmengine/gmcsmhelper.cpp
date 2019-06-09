#include "stdafx.h"
#include "gmcsmhelper.h"
#include <gmutilities.h>
#include <gmgraphicengine.h>
#include <linearmath.h>

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

namespace
{
	struct Frustum
	{
		GMVec3 origin; // Origin of the frustum (and projection).
		GMVec4 orientation; // Unit quaternion representing rotation.

		GMfloat rightSlope; // Positive X slope (X/Z).
		GMfloat leftSlope; // Negative X slope.
		GMfloat topSlope; // Positive Y slope (Y/Z).
		GMfloat bottomSlope; // Negative Y slope.
		GMfloat near, far; // Z of the near plane and far plane.
	};

	void computeFrustumFromProjection(const GMMat4& projection, REF Frustum& frustum)
	{
		// Corners of the projection frustum in homogenous space.
		static GMVec4 s_homogenousPoints[6] =
		{
			{ 1.0f,  0.0f, 1.0f, 1.0f },   // right (at far plane)
			{ -1.0f,  0.0f, 1.0f, 1.0f },   // left
			{ 0.0f,  1.0f, 1.0f, 1.0f },   // top
			{ 0.0f, -1.0f, 1.0f, 1.0f },   // bottom

			{ 0.0f, 0.0f, 0.0f, 1.0f },     // near
			{ 0.0f, 0.0f, 1.0f, 1.0f }      // far
		};

		GMMat4 matInverse = Inverse(projection);

		// Compute the frustum corners in world space.
		GMVec4 points[6];
		for (auto i = 0; i < 6; ++i)
		{
			// Transform point.
			points[i] = s_homogenousPoints[i] * matInverse;
		}
		frustum.origin = GMVec3(0, 0, 0);
		frustum.orientation = GMVec4(0, 0, 0, 0);

		// Compute the slopes.
		points[0] = points[0] * (Reciprocal(SplatZ(points[0])));
		points[1] = points[1] * (Reciprocal(SplatZ(points[1])));
		points[2] = points[2] * (Reciprocal(SplatZ(points[2])));
		points[3] = points[3] * (Reciprocal(SplatZ(points[3])));

		frustum.rightSlope = points[0].getX();
		frustum.leftSlope = points[1].getX();
		frustum.topSlope = points[2].getY();
		frustum.bottomSlope = points[3].getY();
		points[4] = points[4] * Reciprocal(SplatW(points[4]));
		points[5] = points[5] * Reciprocal(SplatW(points[5]));
		frustum.near = points[4].getZ();
		frustum.far = points[5].getZ();
	}


	void createFrustumPointsFromCascadeInterval(const GMCamera& camera, GMfloat intervalBegin, GMfloat intervalEnd, GMVec4* outPoints)
	{
		// 从给定的摄像头中，获取frustum的8个顶点。
		Frustum frustum;
		computeFrustumFromProjection(camera.getProjectionMatrix(), frustum);

		frustum.near = intervalBegin;
		frustum.far = intervalEnd;

		GMVec4 rightTop = { frustum.rightSlope, frustum.topSlope, 1.f, 1.f };
		GMVec4 leftBottom = { frustum.leftSlope, frustum.bottomSlope, 1.f, 1.f };
		GMVec4 near = { frustum.near, frustum.near, frustum.near, 1.f };
		GMVec4 far = { frustum.far, frustum.far, frustum.far, 1.f };

		GMVec4 rightTopNear = near * rightTop;
		GMVec4 rightTopFar = far * rightTop;
		GMVec4 leftBottomNear = near * leftBottom;
		GMVec4 leftBottomFar = far * leftBottom;

		outPoints[0] = rightTopNear;
		outPoints[1] = GMVec4(leftBottomNear.getX(), rightTopNear.getY(), rightTopNear.getZ(), rightTopNear.getW());
		outPoints[2] = leftBottomNear;
		outPoints[3] = GMVec4(rightTopNear.getX(), leftBottomNear.getY(), rightTopNear.getZ(), rightTopNear.getW());

		outPoints[4] = rightTopFar;
		outPoints[5] = GMVec4(leftBottomFar.getX(), rightTopFar.getY(), rightTopFar.getZ(), rightTopFar.getW());
		outPoints[6] = leftBottomFar;
		outPoints[7] = GMVec4(rightTopFar.getX(), leftBottomFar.getY(), rightTopFar.getZ(), rightTopFar.getW());
	}

	void createFrustumPointsFromCascadeInterval2(const GMCamera& camera, GMfloat intervalBegin, GMfloat intervalEnd, GMVec4* outPoints)
	{
		GMfloat tanHalfVFOV = Tan(camera.getFrustum().getParameters().fovy / 2);
		GMfloat tanHalfHFOV = Tan(camera.getFrustum().getParameters().fovy * camera.getFrustum().getParameters().aspect / 2);

		GMfloat xn = intervalBegin * tanHalfHFOV;
		GMfloat xf = intervalEnd * tanHalfHFOV;
		GMfloat yn = intervalBegin * tanHalfVFOV;
		GMfloat yf = intervalEnd * tanHalfVFOV;

		// near face
		outPoints[0] = GMVec4(xn,   yn, intervalBegin, 1.0);
		outPoints[1] = GMVec4(-xn, yn, intervalBegin, 1.0);
		outPoints[2] = GMVec4(xn,  -yn, intervalBegin, 1.0);
		outPoints[3] = GMVec4(-xn, -yn, intervalBegin, 1.0);
		// far face
		outPoints[4] = GMVec4(xf,   yf,intervalEnd, 1.0);
		outPoints[5] = GMVec4(-xf,  yf,intervalEnd, 1.0);
		outPoints[6] = GMVec4(xf,  -yf,intervalEnd, 1.0);
		outPoints[7] = GMVec4(-xf, -yf,intervalEnd, 1.0);
	}
}

void GMCSMHelper::getFrustumIntervals(const GMCamera& camera, const GMShadowSourceDesc& shadowSource, GMCascadeLevel level, GMfloat& frustumIntervalBegin, GMfloat& frustumIntervalEnd)
{
	const auto& frustum = camera.getFrustum();
	GMfloat cameraNearFarRange = frustum.getFar() - frustum.getNear();
	if (level == 0)
		frustumIntervalBegin = 0;
	else
		frustumIntervalBegin = shadowSource.cascadePartitions[level - 1];

	frustumIntervalEnd = shadowSource.cascadePartitions[level];

	frustumIntervalBegin *= cameraNearFarRange;
	frustumIntervalEnd *= cameraNearFarRange;
}

void GMCSMHelper::setOrthoCamera(ICSMFramebuffers* csm, const GMCamera& viewerCamera, const GMShadowSourceDesc& shadowSourceDesc, GMCamera& shadowCamera)
{
	// 通过当前的层级获取间隔
	GMCascadeLevel level = csm->currentLevel();
	GMfloat frustumIntervalBegin = 0;
	GMfloat frustumIntervalEnd = 0;
	getFrustumIntervals(viewerCamera, shadowSourceDesc, level, frustumIntervalBegin, frustumIntervalEnd);

	// 获取frustum顶点
	GMVec4 cornerPoints[8];
	createFrustumPointsFromCascadeInterval(viewerCamera, frustumIntervalBegin, frustumIntervalEnd, cornerPoints);

	// 找到平行投影的最大点和最小点
	GMVec4 tempShadowCameraFrustumCornerPoint;

	static const GMVec4 s_FLTMAX = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };
	static const GMVec4 s_FLTMIN = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
	GMVec4 shadowOrthoMax = s_FLTMIN;
	GMVec4 shadowOrthoMin = s_FLTMAX;
	for (auto i = 0; i < 8; ++i)
	{
		// frustum顶点变换到世界空间
		cornerPoints[i] = cornerPoints[i] * viewerCamera.getInverseViewMatrix();

		// 再从世界空间变换到阴影相机空间
		tempShadowCameraFrustumCornerPoint = cornerPoints[i] * shadowCamera.getViewMatrix();

		// 找到最近的点
		shadowOrthoMax = MaxComponent(tempShadowCameraFrustumCornerPoint, shadowOrthoMax);
		shadowOrthoMin = MinComponent(tempShadowCameraFrustumCornerPoint, shadowOrthoMin);
	}

	shadowCamera.setOrtho(shadowOrthoMin.getX(), shadowOrthoMax.getX(), shadowOrthoMin.getY(), shadowOrthoMax.getY(), shadowOrthoMin.getZ(), shadowOrthoMax.getZ());
}
