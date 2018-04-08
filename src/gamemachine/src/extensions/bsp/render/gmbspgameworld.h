#ifndef __BSPGAMEWORLD_H__
#define __BSPGAMEWORLD_H__
#include <gmcommon.h>
#include "gmgameworld.h"
#include "gmbspgameworldprivate.h"
#include <gmassets.h>
BEGIN_NS

class GMBSPSkyGameObject : public GMGameObject
{
	DECLARE_PRIVATE_AND_BASE(GMBSPSkyGameObject, GMGameObject)

public:
	GMBSPSkyGameObject(const GMShader& shader, const GMVec3& min, const GMVec3& max);
	~GMBSPSkyGameObject();

private:
	void createSkyBox(OUT GMModel** obj);
};

class GMBSPGameWorld : public GMGameWorld
{
	DECLARE_PRIVATE_AND_BASE(GMBSPGameWorld, GMGameWorld);

public:
	GMBSPGameWorld();

public:
	void loadBSP(const GMString& mapName);
	void setSky(AUTORELEASE GMGameObject* sky);
	GMGameObject* getSky();
	Map<GMint, Set<GMBSPEntity*> >& getEntities();
	void addObjectAndInit(AUTORELEASE GMGameObject* obj, bool alwaysVisible);
	void setDefaultLights();
	using GMGameWorld::addObjectAndInit;

public:
	virtual void renderScene() override;

	//renders:
public:
	void drawPolygonFace(GMint polygonFaceNumber);
	void drawMeshFace(GMint meshFaceNumber);
	void drawPatch(GMint patchNumber);

private:
	void clearBuffer();
	void flushBuffer();
	void calculateVisibleFaces();
	void drawAll();
	void drawSky();
	void drawFaces();
	void draw(GMBSP_Render_BiquadraticPatch& biqp);
	void drawAlwaysVisibleObjects();
	template <typename T> bool setMaterialTexture(T& face, REF GMShader& shader);
	void setMaterialLightmap(GMint lightmapid, REF GMShader& shader);
	int isClusterVisible(GMint cameraCluster, GMint testCluster);

	//imports:
private:
	void importBSP();
	void initModels();
	void initShaders();
	void initTextures();
	bool findTexture(const GMString& textureFilename, OUT GMImage** img);
	void initLightmaps();
	void prepareFaces();
	void preparePolygonFace(GMint polygonFaceNumber, GMint drawSurfaceIndex);
	void prepareMeshFace(GMint meshFaceNumber, GMint drawSurfaceIndex);
	void preparePatch(GMint patchNumber, GMint drawSurfaceIndex);
	void prepareEntities();
	GMint calculateLeafNode(const GMVec3& position);

	// this is usually used by BSPShaderLoader, BSPGameWorldEntityReader, physics world
public:
	BSPData& bspData();
	GMBSPRenderData& renderData();
};

END_NS
#endif