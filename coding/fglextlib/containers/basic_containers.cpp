#include "stdafx.h"
#include "basic_containers.h"

VectorContainer::VectorContainer(Ffloat v1, Ffloat v2, Ffloat v3)
	: m_v1(v1)
	, m_v2(v2)
	, m_v3(v3)
{
}

Ffloat VectorContainer::get(Which w)
{
	switch (w)
	{
	case V1:
		return m_v1;
	case V2:
		return m_v2;
	case V3:
		return m_v3;
	}
	return 0;
}