#ifndef __BASIC_CONTAINERS_H__
#define __BASIC_CONTAINERS_H__
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

public:
	Ffloat get(Which) const;

private:
	Ffloat m_v1;
	Ffloat m_v2;
	Ffloat m_v3;
};

typedef VectorContainer Vertices;
typedef VectorContainer VertexNormal;
typedef VectorContainer VertexTexture;

END_NS
#endif