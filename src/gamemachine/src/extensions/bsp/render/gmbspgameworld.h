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
	~GMBSPSkyGameObject();

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
	Map<GMint, Set<GMBSPEntity*> >& getEntities();
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
	void preparePolygonFaceToRenderList(GMint polygonFaceNumber);
	void prepareMeshFaceToRenderList(GMint meshFaceNumber);
	void preparePatchToRenderList(GMint patchNumber);

private:
	void calculateVisibleFaces();
	void prepareAllToRenderList();
	void prepareSkyToRenderList();
	void prepareFacesToRenderList();
	void prepareToRenderList(GMBSP_Render_BiquadraticPatch& biqp);
	void prepareAlwaysVisibleObjects();
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