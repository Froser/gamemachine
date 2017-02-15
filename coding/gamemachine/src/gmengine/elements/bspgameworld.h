#ifndef __BSPGAMEWORLD_H__
#define __BSPGAMEWORLD_H__
#include "common.h"
#include "gameworld.h"
#include "bspgameworldprivate.h"
BEGIN_NS

class BSPGameWorld : public GameWorld
{
	DEFINE_PRIVATE(BSPGameWorld);

public:
	void loadBSP(const char* bspPath);

private:
	void importBSP();
};

END_NS
#endif