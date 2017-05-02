#ifndef __MODEL_H__
#define __MODEL_H__
#include "common.h"
#include "utilities/vmath.h"
BEGIN_NS

#define NAME_MAX 64

struct Model
{
	char classname[NAME_MAX];
	char model[NAME_MAX];
	bool create;
	vmath::vec3 extents;
};

END_NS
#endif