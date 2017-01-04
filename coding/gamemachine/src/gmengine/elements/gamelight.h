#ifndef __GAMELIGHT_H__
#define __GAMELIGHT_H__
#include "common.h"
BEGIN_NS

class GameWorld;
class GameLight
{
public:
	GameLight();
	virtual ~GameLight();

public:
	void setRange(GMfloat range);
	GMfloat getRange();

	void setPosition(const GMfloat pos[3]);
	GMfloat* getPosition();

	void setColor(const GMfloat color[3]);
	GMfloat* getColor();
public:
	virtual void activateLight() = 0;
	virtual bool isAvailable() = 0;

	void setWorld(GameWorld* world);
	GameWorld* getWorld();

private:
	GMfloat m_range;
	GMfloat m_position[3];
	GMfloat m_color[3];
	GameWorld* m_world;
};

class AmbientLight : public GameLight
{
public:
	virtual void activateLight() override;
	virtual bool isAvailable() override;
};

END_NS
#endif