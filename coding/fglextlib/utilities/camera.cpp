#include "stdafx.h"
#include <cmath>
#include <gl/GLU.h>
#include "camera.h"
#include "assert.h"

static const Ffloat PI = std::acos(-1.0f);
static const Ffloat SC = PI / 2;
inline Ffloat rad(Ffloat deg)
{
	return PI * deg / 180;
}

Camera::Camera()
	: m_lookAtRad(0)
	, m_lookUpRad(0)
	, m_sensibility(1)
	, m_positionX(0)
	, m_positionY(0)
	, m_positionZ(0)
{
}

void Camera::setPosition(Ffloat x, Ffloat y, Ffloat z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
}

void Camera::lookRight(Ffloat degree)
{
	m_lookAtRad += rad(degree);
}

void Camera::lookUp(Ffloat degree)
{
	m_lookUpRad += rad(degree);
	if (m_lookUpRad > SC)
		m_lookUpRad = SC;
	else if (m_lookUpRad < -SC)
		m_lookUpRad = -SC;
}

void Camera::moveFront(Ffloat distance)
{
	m_positionY += distance * std::sin(m_lookUpRad);
	Ffloat l = distance * std::cos(m_lookUpRad);
	m_positionX += l * std::sin(m_lookAtRad);
	m_positionZ -= l * std::cos(m_lookAtRad);
}

void Camera::moveRight(Ffloat distance)
{
	m_positionX += distance * std::cos(m_lookAtRad);
	m_positionZ += distance * std::sin(m_lookAtRad);
}

void Camera::setSensibility(Ffloat sensibility)
{
	m_sensibility = sensibility;
}

CameraLookAt Camera::getCameraLookAt()
{
	CameraLookAt lookAt;
	lookAt.lookAt_y = std::sin(m_lookUpRad);
	Ffloat l = std::cos(m_lookUpRad);
	lookAt.lookAt_x = l * std::sin(m_lookAtRad);
	lookAt.lookAt_z = -l * std::cos(m_lookAtRad);

	lookAt.lookUp_y = std::cos(m_lookUpRad);
	Ffloat _l = std::sin(m_lookUpRad);
	lookAt.lookUp_x = _l * std::sin(m_lookAtRad);
	lookAt.lookUp_z = _l * std::cos(m_lookAtRad);

	lookAt.position_x = m_positionX;
	lookAt.position_y = m_positionY;
	lookAt.position_z = m_positionZ;
	return lookAt;
}

void Camera::mouseInitReaction(int windowPosX, int windowPosY, int windowWidth, int WindowHeight)
{
#ifdef _WINDOWS
	const int centerX = windowPosX + windowWidth / 2;
	const int centerY = windowPosY + WindowHeight / 2;
	::SetCursorPos(centerX, centerY);
	::ShowCursor(FALSE);
#endif
}

void Camera::mouseReact(int windowPosX, int windowPosY, int windowWidth, int WindowHeight)
{
#ifdef _WINDOWS
	const int centerX = windowPosX + windowWidth / 2;
	const int centerY = windowPosY + WindowHeight / 2;

	POINT pos;
	::GetCursorPos(&pos);
	m_currentMouseX = pos.x;
	m_currentMouseY = pos.y;
	int deltaX = m_currentMouseX - centerX, deltaY = m_currentMouseY - centerY;
	// 先不考虑灵敏度
	lookRight(deltaX * m_sensibility);
	lookUp(-deltaY * m_sensibility);
	::SetCursorPos(centerX, centerY);
#endif
}

void CameraUtility::lookAt(Camera& camera)
{
	CameraLookAt c = camera.getCameraLookAt();
	gluLookAt(c.position_x, c.position_y, c.position_z, c.lookAt_x + c.position_x, c.lookAt_y + c.position_y, c.lookAt_z + c.position_z, c.lookUp_x, c.lookUp_y, c.lookUp_z);
}
