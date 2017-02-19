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
	BSPGameWorld(GMfloat scaling);

public:
	void loadBSP(const char* bspPath);

public:
	virtual void renderGameWorld() override;

	//renders:
private:
	void calculateVisibleFaces();
	void drawFaces();
	int calculateCameraLeaf(const btVector3& cameraPosition);
	int isClusterVisible(int cameraCluster, int testCluster);

	//imports:
private:
	void importBSP();
	void importPlayer();
	void importWorldSpawn();
	void importLeafs();
	void addConvexVerticesCollider(btAlignedObjectArray<btVector3>& vertices, bool isEntity, const btVector3& entityTargetLocation);
};

END_NS
#endif