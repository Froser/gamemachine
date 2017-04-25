#ifndef __MODEL_H__
#define __MODEL_H__
#include "common.h"
BEGIN_NS

#define NAME_MAX 64

struct Model
{
	char classname[NAME_MAX];
	bool create;
};

END_NS
#endif