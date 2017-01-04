#include "stdafx.h"
#include "gamelight.h"
#include "gameworld.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmengine/controller/light.h"

GameLight::GameLight()
{
}

GameLight::~GameLight()
{
}

void GameLight::setRange(GMfloat range)
{
	m_range = range;
}

GMfloat GameLight::getRange()
{
	return m_range;
}

void GameLight::setPosition(const GMfloat pos[3])
{
	m_position[0] = pos[0];
	m_position[1] = pos[1];
	m_position[2] = pos[2];
}

GMfloat* GameLight::getPosition()
{
	return m_position;
}

void GameLight::setColor(const GMfloat color[3])
{
	m_color[0] = color[0];
	m_color[1] = color[1];
	m_color[2] = color[2];
}

GMfloat* GameLight::getColor()
{
	return m_color;
}

void GameLight::setWorld(GameWorld* world)
{
	m_world = world;
}

GameWorld* GameLight::getWorld()
{
	return m_world;
}

void AmbientLight::activateLight()
{
	ILightController& lightCtrl = getWorld()->getGraphicEngine()->getLightController();
	lightCtrl.setLightPosition(getPosition());
	lightCtrl.setAmbient(getColor());
}

bool AmbientLight::isAvailable()
{
	return true;
}
