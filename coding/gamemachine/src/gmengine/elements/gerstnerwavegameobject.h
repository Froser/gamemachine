#ifndef __GERSTNERWAVEGAMEOBJECT_H__
#define __GERSTNERWAVEGAMEOBJECT_H__
#include "common.h"
#include "hallucinationgameobject.h"
BEGIN_NS

class GerstnerWaveGameObject : public HallucinationGameObject
{
public:
	GerstnerWaveGameObject(const btVector3& extents, GMfloat Q, GMfloat D, GMfloat A);

private:
	btVector3 m_extents;
	GMfloat m_Q;
	GMfloat m_D;
	GMfloat m_A;
};

END_NS
#endif