#ifndef __BSPGAMEWORLD_H__
#define __BSPGAMEWORLD_H__
#include <gmcommon.h>
#include "gmgameworld.h"
#include "gmbspgameworldprivate.h"
#include <gmassets.h>
BEGIN_NS

class GMBSPSkyGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMBSPSkyGameObject, GMGameObject)

public:
	GMBSPSkyGameObject(const GMShader& shader, const GMVec3& min, const GMVec3& max);

private:
	void createSkyBox(OUT GMModel** obj);
};

class GMBSPGameWorld : public GMGameWorld
{
	GM_DECLARE_PRIVATE_AND_BASE(GMBSPGameWorld, GMGameWorld);

public:
	GMBSPGameWorld(const IRenderContext* context);

public:
	void loadBSP(const GMString& mapName);
	void setSky(AUTORELEASE GMGameObject* sky);
	GMGameObject* getSky();
	Map<GMint32, Set<GMBSPEntity*> >& getEntities();
	void addObjectAndInit(AUTORELEASE GMGameObject* obj, bool alwaysVisible);
	void setDefaultLights();
	void setSprite(GMSpriteGameObject* sprite);
	GMSpriteGameObject* getSprite();

	using GMGameWorld::addObjectAndInit;

	void setRenderConfig(gm::GMBSPRenderConfigs config, const GMVariant& value);
	const GMVariant& getRenderConfig(gm::GMBSPRenderConfigs config);

public:
	virtual void renderScene() override;

	//renders:
public:
	void preparePolygonFaceToRenderList(GMint32 polygonFaceNumber);
	void prepareMeshFaceToRenderList(GMint32 meshFaceNumber);
	void preparePatchToRenderList(GMint32 patchNumber);

private:
	void calculateVisibleFaces();
	void prepareAllToRenderList();
	void prepareSkyToRenderList();
	void prepareFacesToRenderList();
	void prepareToRenderList(GMBSP_Render_BiquadraticPatch& biqp);
	void prepareAlwaysVisibleObjects();
	template <typename T> bool setMaterialTexture(T& face, REF GMShader& shader);
	void setMaterialLightmap(GMint32 lightmapid, REF GMShader& shader);
	int isClusterVisible(GMint32 cameraCluster, GMint32 testCluster);

	//imports:
private:
	void importBSP();
	void initModels();
	void initShaders();
	void initTextures();
	bool findTexture(const GMString& textureFilename, OUT GMImage** img);
	void initLightmaps();
	void prepareFaces();
	void preparePolygonFace(GMint32 polygonFaceNumber, GMint32 drawSurfaceIndex);
	void prepareMeshFace(GMint32 meshFaceNumber, GMint32 drawSurfaceIndex);
	void preparePatch(GMint32 patchNumber, GMint32 drawSurfaceIndex);
	void prepareEntities();
	GMint32 calculateLeafNode(const GMVec3& position);

	// this is usually used by BSPShaderLoader, BSPGameWorldEntityReader, physics world
public:
	BSPData& bspData();
	GMBSPRenderData& renderData();
};

END_NS
#endif