#ifndef __BSPGAMEWORLD_H__
#define __BSPGAMEWORLD_H__
#include "common.h"
#include "gameworld.h"
#include "bspgameworldprivate.h"
#include "gmengine/controllers/resource_container.h"
BEGIN_NS

struct Material;
class BSPGameWorld : public GameWorld
{
	DEFINE_PRIVATE(BSPGameWorld);
public:
	BSPGameWorld();

public:
	void loadBSP(const char* bspPath);
	void setSky(AUTORELEASE GameObject* sky);
	GameObject* getSky();

public:
	virtual void renderGameWorld() override;

	//renders:
private:
	void updateCamera();
	void calculateVisibleFaces();
	void drawAll();
	void drawSky();
	void drawFaces();
	void drawFace(GMint idx);
	void drawPolygonFace(int polygonFaceNumber);
	void drawMeshFace(int meshFaceNumber);
	void drawPatch(int patchNumber);
	void draw(BSP_Drawing_BiquadraticPatch& biqp, Material& material);
	template <typename T> bool setMaterialTexture(T face, REF Material& m);
	void setMaterialLightmap(GMint lightmapid, REF Material& m);
	int calculateCameraLeaf(const vmath::vec3& cameraPosition);
	int isClusterVisible(int cameraCluster, int testCluster);

	//imports:
private:
	void importBSP();
	void initShaders();
	void initTextures();
	bool findTexture(const char* textureFilename, OUT Image** img);
	void initLightmaps();
	void importEntities();

	// this is ususally used by BSPShaderLoader, BSPGameWorldEntityReader
public:
	const char* bspWorkingDirectory();
	BSPData& bspData();
};

END_NS
#endif