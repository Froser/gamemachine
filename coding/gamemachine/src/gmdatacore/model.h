#ifndef __MODEL_H__
#define __MODEL_H__
#include <gmcommon.h>
#include <linearmath.h>
BEGIN_NS

GM_ALIGNED_STRUCT(Model)
{
	linear_math::Vector3 extents;
	char classname[64];
	char model[64];
	bool create;
};

END_NS
#endif
