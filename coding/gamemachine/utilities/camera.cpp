#include "stdafx.h"
#include "camera.h"
#include <cmath>
#include <gl/GLU.h>
#include "assert.h"
#include "io/mouse.h"

static GMfloat PI()
{
	static const GMfloat _PI = std::acos(-1.0f);
	return _PI;
}

static GMfloat SC()
{
	static GMfloat _SC = PI() / 2;
	return _SC;
}

inline GMfloat rad(GMfloat deg)
{
	return PI() * deg / 180;
}

Camera::Camera()
	: m_lookAtRad(0)
	, m_lookUpRad(0)
	, m_sensibility(1)
	, m_positionX(0)
	, m_positionY(0)
	, m_positionZ(0)
	, m_lookUpLimitRad(SC() - rad(3))
	, m_type(FreeCamera)
{
}

void Camera::setPosition(GMfloat x, GMfloat y, GMfloat z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
}

void Camera::setType(Type type)
{
	m_type = type;
}

void Camera::setLookUpLimitDegree(GMfloat deg)
{
	m_lookUpLimitRad = SC() - rad(deg);
}

void Camera::lookRight(GMfloat degree)
{
	m_lookAtRad += rad(degree);
}

void Camera::lookUp(GMfloat degree)
{
	m_lookUpRad += rad(degree);
	if (m_lookUpRad > m_lookUpLimitRad)
		m_lookUpRad = m_lookUpLimitRad;
	else if (m_lookUpRad < -m_lookUpLimitRad)
		m_lookUpRad = -m_lookUpLimitRad;
}

void Camera::moveFront(GMfloat distance)
{
	if (m_type == FreeCamera)
		m_positionY += distance * std::sin(m_lookUpRad);
	GMfloat l = distance * std::cos(m_lookUpRad);
	m_positionX += l * std::sin(m_lookAtRad);
	m_positionZ -= l * std::cos(m_lookAtRad);
}

void Camera::moveRight(GMfloat distance)
{
	m_positionX += distance * std::cos(m_lookAtRad);
	m_positionZ += distance * std::sin(m_lookAtRad);
}

void Camera::moveTo(GMfloat x, GMfloat y, GMfloat z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
}

void Camera::setSensibility(GMfloat sensibility)
{
	m_sensibility = sensibility;
}

CameraLookAt Camera::getCameraLookAt()
{
	CameraLookAt lookAt;
	lookAt.lookAt_y = std::sin(m_lookUpRad);
	GMfloat l = std::cos(m_lookUpRad);
	lookAt.lookAt_x = l * std::sin(m_lookAtRad);
	lookAt.lookAt_z = -l * std::cos(m_lookAtRad);

	lookAt.position_x = m_positionX;
	lookAt.position_y = m_positionY;
	lookAt.position_z = m_positionZ;
	return lookAt;
}

void Camera::mouseInitReaction(int windowPosX, int windowPosY, int windowWidth, int WindowHeight)
{
	const int centerX = windowPosX + windowWidth / 2;
	const int centerY = windowPosY + WindowHeight / 2;
	Mouse::setCursorPosition(centerX, centerY);
	Mouse::showCursor(false);
}

void Camera::mouseReact(int windowPosX, int windowPosY, int windowWidth, int WindowHeight)
{
	const int centerX = windowPosX + windowWidth / 2;
	const int centerY = windowPosY + WindowHeight / 2;

	int x = 0, y = 0;
	Mouse::getCursorPosition(&x, &y);
	m_currentMouseX = x;
	m_currentMouseY = y;
	int deltaX = m_currentMouseX - centerX, deltaY = m_currentMouseY - centerY;

	lookRight(deltaX * m_sensibility);
	lookUp(-deltaY * m_sensibility);
	Mouse::setCursorPosition(centerX, centerY);
}

void CameraUtility::fglextlib_gl_LookAt(Camera& camera)
{
	CameraLookAt c = camera.getCameraLookAt();
	gluLookAt(c.position_x, c.position_y, c.position_z,
		c.lookAt_x + c.position_x, c.lookAt_y + c.position_y, c.lookAt_z + c.position_z,
		0, 1, 0);
}
