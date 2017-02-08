#include "stdafx.h"
#include "gamelight.h"
#include "gameworld.h"
#include "gmengine/controller/graphic_engine.h"

GameLight::GameLight()
	: m_shadowSource(false)
{
}

GameLight::~GameLight()
{
}

void GameLight::setId(GMuint id)
{
	m_id = id;
}

GMuint GameLight::getId()
{
	return m_id;
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

void GameLight::setShadowSource(bool shadowSource)
{
	m_shadowSource = shadowSource;
}

bool GameLight::getShadowSource()
{
	return m_shadowSource;
}