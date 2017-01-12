#include "stdafx.h"
#include "gerstnerwavegameobject.h"

GerstnerWaveGameObject::GerstnerWaveGameObject(const btVector3& extents, GMfloat Q, GMfloat D, GMfloat A)
	: m_extents(extents)
	, m_Q(Q)
	, m_D(D)
	, m_A(A)
	, HallucinationGameObject(nullptr)
{

}