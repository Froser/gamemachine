#ifndef __BSPGAMEWORLD_H__
#define __BSPGAMEWORLD_H__
#include <gmcommon.h>
#include <gmassets.h>
#include <gmgameworld.h>
#include "../data/gmbsp.h"
#include "../data/gmbsp_render.h"

BEGIN_NS

enum class GMBSPRenderConfigs
{
	DrawSkyOnly_Bool,
	CalculateFace_Bool,
	Max,
};

GM_DEFINE_CONFIG(GMBSPRenderConfigs, GMBSPRenderConfig);

GM_PRIVATE_CLASS(GMBSPSkyGameObject);
class GM_EXPORT GMBSPSkyGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE(GMBSPSkyGameObject)
	GM_DECLARE_BASE(GMGameObject)

public:
	GMBSPSkyGameObject(const GMShader& shader, const GMVec3& min, const GMVec3& max);
	~GMBSPSkyGameObject();

private:
	void createSkyBox(OUT GMModel** obj);
};

struct GMBSPEntity;
GM_PRIVATE_CLASS(GMBSPGameWorld);
class GM_EXPORT GMBSPGameWorld : public GMGameWorld
{
	GM_DECLARE_PRIVATE(GMBSPGameWorld);
	GM_DECLARE_BASE(GMGameWorld)

public:
	GMBSPGameWorld(const IRenderContext* context);
	~GMBSPGameWorld();

public:
	void loadBSP(const GMString& mapName);
	void loadBSP(const GMBuffer& buffer);
	void setSky(AUTORELEASE GMGameObject* sky);
	GMGameObject* getSky();
	Map<GMint32, Set<GMBSPEntity*> >& getEntities();
	void addObjectAndInit(AUTORELEASE GMGameObject* obj, bool alwaysVisible);
	void setSprite(GMSpriteGameObject* sprite);
	GMSpriteGameObject* getSprite();

	using GMGameWorld::addObjectAndInit;

	void setRenderConfig(GMBSPRenderConfigs config, const GMVariant& value);
	const GMVariant& getRenderConfig(GMBSPRenderConfigs config);

public:
	virtual void setDefaultLights();
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
	GMBSPData& bspData();
	GMBSPRenderData& renderData();
};

END_NS
#endif