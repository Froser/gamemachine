#ifndef __GERSTNERWAVEGAMEOBJECT_H__
#define __GERSTNERWAVEGAMEOBJECT_H__
#include "common.h"
#include "hallucinationgameobject.h"
#include "gmdatacore\object.h"
BEGIN_NS

class GerstnerWaveGameObject : public HallucinationGameObject
{
public:
	GerstnerWaveGameObject(const Material& material);

public:
	virtual void getReadyForRender(DrawingList& list) override;

private:
	Object* createCoreObject();
	void initAll();
	void initWave();
	void calcWave();

private:
	Material m_material;
	GMfloat m_lastTick;
};

END_NS
#endif