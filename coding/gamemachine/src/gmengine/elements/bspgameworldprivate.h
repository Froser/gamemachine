#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore\bsp.h"
BEGIN_NS

struct BSPGameWorldPrivate
{
	BSP bsp;
	GMfloat scaling;
};

END_NS
#endif