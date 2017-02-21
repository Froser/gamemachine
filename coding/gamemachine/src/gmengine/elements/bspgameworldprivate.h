#ifndef __BSPGAMEWORLDPRIVATE_H__
#define __BSPGAMEWORLDPRIVATE_H__
#include "common.h"
#include "gmdatacore\bsp.h"
#include <map>
#include "gmengine\controller\graphic_engine.h"
BEGIN_NS

class GameObject;
struct BSPGameWorldPrivate
{
	BSPGameWorldPrivate();

	BSP bsp;
	std::string bspWorkingDirectory;

	std::map<BSP_Drawing_BiquadraticPatch*, GameObject*> biquadraticPatchObjects;
	std::map<BSP_Drawing_PolygonFace*, GameObject*> polygonFaceObjects;
	std::map<BSP_Drawing_MeshFace*, GameObject*> meshFaceObjects;

	// list to be drawn each frame
	DrawingList drawingList;
};

END_NS
#endif