#ifndef __PHOTOGRAPHER_H__
#define __PHOTOGRAPHER_H__
#include "common.h"
BEGIN_NS

struct CameraLookAt
{
	GMfloat lookAt_x, lookAt_y, lookAt_z;
	GMfloat position_x, position_y, position_z;
};

class Camera
{
public:
	enum Type
	{
		FreeCamera,
		PhysicalCamera,
	};

public:
	Camera();

public:
	void setType(Type type);
	void setPosition(GMfloat x, GMfloat y, GMfloat z);
	void setLookUpLimitDegree(GMfloat deg);
	void lookRight(GMfloat degree);
	void lookUp(GMfloat degree);
	void moveFront(GMfloat distance);
	void moveRight(GMfloat distance);
	void moveTo(GMfloat x, GMfloat y, GMfloat z);

	void setSensibility(GMfloat sensibility);
	CameraLookAt getCameraLookAt();

	void mouseInitReaction(int windowPosX, int windowPosY, int windowWidth, int WindowHeight);
	void mouseReact(int windowPosX, int windowPosY, int windowWidth, int WindowHeight);

private:
	Type m_type;
	int m_currentMouseX, m_currentMouseY;
	GMfloat m_sensibility;
	GMfloat m_lookAtRad;
	GMfloat m_lookUpRad;
	GMfloat m_positionX, m_positionY, m_positionZ;
	GMfloat m_lookUpLimitRad;
};

struct CameraUtility
{
	static void fglextlib_gl_LookAt(Camera& camera);
};

END_NS
#endif