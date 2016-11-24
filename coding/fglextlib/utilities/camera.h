#ifndef __PHOTOGRAPHER_H__
#include "common.h"
BEGIN_NS

struct CameraLookAt
{
	Ffloat lookAt_x, lookAt_y, lookAt_z;
	Ffloat position_x, position_y, position_z;
};

class Camera
{
public:
	Camera();

public:
	void setPosition(Ffloat x, Ffloat y, Ffloat z);
	void setLookUpLimitDegree(Ffloat deg);
	void lookRight(Ffloat degree);
	void lookUp(Ffloat degree);
	void moveFront(Ffloat distance);
	void moveRight(Ffloat distance);

	void setSensibility(Ffloat sensibility);
	CameraLookAt getCameraLookAt();

	void mouseInitReaction(int windowPosX, int windowPosY, int windowWidth, int WindowHeight);
	void mouseReact(int windowPosX, int windowPosY, int windowWidth, int WindowHeight);

private:
	int m_currentMouseX, m_currentMouseY;
	Ffloat m_sensibility;
	Ffloat m_lookAtRad;
	Ffloat m_lookUpRad;
	Ffloat m_positionX, m_positionY, m_positionZ;
	Ffloat m_lookUpLimitRad;
};

struct CameraUtility
{
	static void fglextlib_gl_LookAt(Camera& camera);
};

END_NS
#endif