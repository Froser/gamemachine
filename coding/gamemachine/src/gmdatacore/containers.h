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
	VectorContainer(GMfloat v1, GMfloat v2, GMfloat v3);

public:
	GMfloat get(Which) const;

private:
	GMfloat m_v1;
	GMfloat m_v2;
	GMfloat m_v3;
};

typedef VectorContainer Vertices;
typedef VectorContainer VertexNormal;
typedef VectorContainer VertexTexture;

END_NS
#endif