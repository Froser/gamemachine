#ifndef __GMPRIMITIVECREATOR_H__
#define __GMPRIMITIVECREATOR_H__
#include "common.h"
#include "gmdatacore/gmmodel.h"
BEGIN_NS

class GMModel;
struct GMPrimitiveCreator
{
	static void createCube(GMfloat extents[3], OUT GMModel** obj, GMMeshType type = GMMeshType::Model);
	static void createPlane(GMfloat extents[3], OUT GMModel** obj, GMMeshType type = GMMeshType::Model);
};

END_NS
#endif