#ifndef __BASIC_CONTAINERS_H__
#include "common.h"
BEGIN_NS

class VectorContainer
{
public:
	enum Which
	{
		V1,
		V2,
		V3,
	};

public:
	VectorContainer(Ffloat v1, Ffloat v2, Ffloat v3);

private:
	Ffloat get(Which);

private:
	Ffloat m_v1;
	Ffloat m_v2;
	Ffloat m_v3;
};

typedef VectorContainer Vertices;
typedef VectorContainer VertexNormal;

END_NS
#endif