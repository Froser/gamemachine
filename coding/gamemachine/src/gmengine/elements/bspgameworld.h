#ifndef __BSPGAMEWORLD_H__
#define __BSPGAMEWORLD_H__
#include "common.h"
#include "gameworld.h"
#include "bspgameworldprivate.h"
#include "gmengine\controller\resource_container.h"
BEGIN_NS

struct Material;
class BSPGameWorld : public GameWorld
{
	DEFINE_PRIVATE(BSPGameWorld);
public:
	BSPGameWorld();

public:
	void loadBSP(const char* bspPath);

public:
	virtual void renderGameWorld() override;

	//renders:
private:
	void calculateVisibleFaces();
	void drawFaces();
	void drawFace(GMint idx);
	void drawPolygonFace(int polygonFaceNumber);
	void drawMeshFace(int meshFaceNumber);
	void drawPatch(int patchNumber);
	void draw(BSP_Drawing_BiquadraticPatch& biqp, Material& material);
	bool setMaterialTexture(ID textureid, REF Material& m);
	void setMaterialLightmap(ID lightmapid, REF Material& m);
	int calculateCameraLeaf(const btVector3& cameraPosition);
	int isClusterVisible(int cameraCluster, int testCluster);

	//imports:
private:
	void importBSP();
	void initTextures();
	void initLightmaps();
	void adjustLightmapGamma();
	void importPlayer();
	void importWorldSpawn();
};

END_NS
#endif