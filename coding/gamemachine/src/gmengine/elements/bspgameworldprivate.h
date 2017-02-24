#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include <map>
#include "gmengine/controllers/graphic_engine.h"
BEGIN_NS

class GameObject;
class BSPGameWorld;
struct BSPGameWorldPrivate
{
	BSP bsp;
	std::string bspWorkingDirectory;

	std::map<BSP_Drawing_BiquadraticPatch*, GameObject*> biquadraticPatchObjects;
	std::map<BSP_Drawing_PolygonFace*, GameObject*> polygonFaceObjects;
	std::map<BSP_Drawing_MeshFace*, GameObject*> meshFaceObjects;

	// list to be drawn each frame
	DrawingList drawingList;
};

struct BSPGameWorldEntityReader
{
	static void import(const BSPEntity& entity, BSPGameWorld* world);
};

END_NS
#endif