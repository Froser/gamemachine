#ifndef __MODEL_H__
#define __MODEL_H__
#include "common.h"
#include "utilities/linearmath.h"
BEGIN_NS

struct Model
{
	linear_math::Vector3 extents;
	bool create;
	char classname[64];
	char model[64];
};

END_NS
#endif
