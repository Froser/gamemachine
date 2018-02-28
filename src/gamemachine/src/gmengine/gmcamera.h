#ifndef __GMCAMERA_H__
#define __GMCAMERA_H__
#include <gmcommon.h>
#include <tools.h>
#include <gmdxincludes.h>
#include <gmcom.h>

BEGIN_NS

//Camera
struct GMMVPMatrix
{
	GMMat4 projMatrix;
	GMMat4 viewMatrix;
	GMMat4 modelMatrix; // 在绘制的时候，将物体的Model transform放到此处
};

GM_ALIGNED_STRUCT(GMCameraLookAt)
{
	GMCameraLookAt() = default;
	GMCameraLookAt(const GMVec3& _lookAt, const GMVec3& _position, const GMVec3& _up)
		: lookAt(_lookAt)
		, position(_position)
		, up(_up)
	{
	}

	GMCameraLookAt(const GMVec3& _lookAt, const GMVec3& _position)
		: lookAt(_lookAt)
		, position(_position)
	{
	}

	GMVec3 lookAt = Zero<GMVec3>(); //!< 摄像机朝向，单位向量指示其方向
	GMVec3 position = Zero<GMVec3>(); //!< 摄像机位置
	GMVec3 up = GMVec3(0, 1, 0);
};

inline GMMat4 getViewMatrix(const GMCameraLookAt& lookAt)
{
	return LookAt(lookAt.position, lookAt.lookAt + lookAt.position, lookAt.up);
}

//Frustum
enum class GMFrustumType
{
	Perspective,
	Orthographic,
};

GM_PRIVATE_OBJECT(GMFrustum)
{
	GMFrustumType type = GMFrustumType::Perspective;

	union
	{
		struct
		{
			GMfloat fovy;
			GMfloat aspect;
		};

		struct
		{
			GMfloat left;
			GMfloat right;
			GMfloat bottom;
			GMfloat top;
		};
	};
	GMfloat n;
	GMfloat f;

	GMMVPMatrix mvpMatrix;

#if GM_USE_DX11
	//DirectX
	GMComPtr<ID3D11Buffer> dxMatrixBuffer;
#endif
};

class GMSpriteGameObject;
class GMFrustum : public GMObject
{
	DECLARE_PRIVATE(GMFrustum)

public:
	GMFrustum() = default;
	void setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);
	void setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);

public:
	void updateViewMatrix(const GMMat4& viewMatrix);

	//! 获取平截头体的6个平面方程。
	/*!
	  获取平截头体的6个平面方程，法线方向朝外。
	  \param planes 得到的平面方程。
	*/
	void getPlanes(GMFrustumPlanes& planes);

public:
	inline GMFrustumType getType() { D(d); return d->type; }
	inline GMfloat getNear() { D(d); return d->n; }
	inline GMfloat getFar() { D(d); return d->f; }
	inline GMfloat getFovy() { D(d); GM_ASSERT(getType() == GMFrustumType::Perspective); return d->fovy; }

public:
	const GMMat4& getProjectionMatrix();
	const GMMat4& getViewMatrix();

#if GM_USE_DX11
	void setDxMatrixBuffer(GMComPtr<ID3D11Buffer> buffer);
	GMComPtr<ID3D11Buffer> getDxMatrixBuffer();
#endif

public:
	static bool isBoundingBoxInside(const GMFrustumPlanes& planes, const GMVec3 (&vertices)[8]);
};

GM_PRIVATE_OBJECT(GMCamera)
{
	GMFrustum frustum;
	GMCameraLookAt lookAt;
};

class GMCamera
{
	DECLARE_PRIVATE_NGO(GMCamera)

public:
	GMCamera();

public:
	void setPerspective(GMfloat fovy, GMfloat aspect, GMfloat n, GMfloat f);
	void setOrtho(GMfloat left, GMfloat right, GMfloat bottom, GMfloat top, GMfloat n, GMfloat f);

	void synchronize(GMSpriteGameObject* gameObject);
	void synchronizeLookAt();

	void lookAt(const GMCameraLookAt& lookAt);
	GMFrustum& getFrustum() { D(d); return d->frustum; }

	//! 获取从屏幕出发，变换到世界的一条射线
	/*!
	  将屏幕上的一点，变换到世界坐标。世界坐标的深度值取远平面的值。此方法目前只对透视投影有效。
	  \param x 屏幕上的点的x坐标。
	  \param y 屏幕上的点的y坐标。
	  \return 世界坐标。
	*/
	GMVec3 getRayToWorld(GMint x, GMint y) const;

public:
	inline const GMMat4& getProjectionMatrix() { D(d); return getFrustum().getProjectionMatrix(); }
	inline const GMMat4& getViewMatrix() { D(d); return getFrustum().getViewMatrix(); }
	inline const GMCameraLookAt& getLookAt() { D(d); return d->lookAt; }
};

END_NS
#endif