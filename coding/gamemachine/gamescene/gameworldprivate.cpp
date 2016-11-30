#include "stdafx.h"
#include "gameworldprivate.h"

GameWorldPrivate::GameWorldPrivate()
	: m_gravity(0, -10, 0)
{

}

void GameWorldPrivate::setGravity(GMfloat x, GMfloat y, GMfloat z)
{
	m_gravity.setX(x);
	m_gravity.setY(x);
	m_gravity.setZ(z);
}