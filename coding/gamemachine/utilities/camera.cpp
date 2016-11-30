#include "stdafx.h"
#include "camera.h"
#include <cmath>
#include <gl/GLU.h>
#include "assert.h"
#include "io/mouse.h"

static Ffloat PI()
{
	static const Ffloat _PI = std::acos(-1.0f);
	return _PI;
}

static Ffloat SC()
{
	static Ffloat _SC = PI() / 2;
	return _SC;
}

inline Ffloat rad(Ffloat deg)
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
{
}

void Camera::setPosition(Ffloat x, Ffloat y, Ffloat z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
}

void Camera::setLookUpLimitDegree(Ffloat deg)
{
	m_lookUpLimitRad = SC() - rad(deg);
}

void Camera::lookRight(Ffloat degree)
{
	m_lookAtRad += rad(degree);
}

void Camera::lookUp(Ffloat degree)
{
	m_lookUpRad += rad(degree);
	if (m_lookUpRad > m_lookUpLimitRad)
		m_lookUpRad = m_lookUpLimitRad;
	else if (m_lookUpRad < -m_lookUpLimitRad)
		m_lookUpRad = -m_lookUpLimitRad;
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
