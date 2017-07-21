#ifndef __GMPRIMITIVECREATOR_H__
#define __GMPRIMITIVECREATOR_H__
#include "common.h"
#include "gmdatacore/object.h"
BEGIN_NS

class Object;
struct GMPrimitiveCreator
{
	static void createCube(GMfloat extents[3], OUT Object** obj, GMMeshType type = GMMeshType::Normal);
	static void createPlane(GMfloat extents[3], OUT Object** obj, GMMeshType type = GMMeshType::Normal);
};

END_NS
#endif