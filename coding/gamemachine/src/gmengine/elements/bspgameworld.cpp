#include "stdafx.h"
#include "bspgameworld.h"

void BSPGameWorld::loadBSP(const char* bspPath)
{
	D(d);
	d.bsp.loadBsp(bspPath);
	importBSP();
}

void BSPGameWorld::importBSP()
{
	D(d);
	const BSPData& bsp = d.bsp.bspData();

	float playstartf[3] = { 0,0,100 };
	if (d.bsp.findVectorByName(&playstartf[0], "info_player_start"))
	{
		printf("found playerstart\n");
	}
	else
	{
		if (d.bsp.findVectorByName(&playstartf[0], "info_player_deathmatch"))
		{
			printf("found deatchmatch start\n");
		}
	}
}