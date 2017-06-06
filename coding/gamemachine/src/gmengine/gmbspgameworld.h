#ifndef __BSPGAMEWORLD_H__
#define __BSPGAMEWORLD_H__
#include "common.h"
#include "gmgameworld.h"
#include "gmbspgameworldprivate.h"
#include "gmengine/controllers/resource_container.h"
#include "gmdatacore/bsp/gmbsp.h"
BEGIN_NS

class GMBSPGameWorld : public GMGameWorld
{
	DECLARE_PRIVATE(GMBSPGameWorld);

public:
	GMBSPGameWorld();

public:
	void loadBSP(const char* mapName);
	void setSky(AUTORELEASE GMGameObject* sky);
	GMGameObject* getSky();
	void appendObjectAndInit(AUTORELEASE GMGameObject* obj, bool alwaysVisible);
	std::map<GMint, std::set<GMBSPEntity*> >& getEntities();
	using GMGameWorld::appendObjectAndInit;

public:
	virtual void renderGameWorld() override;
	virtual GMPhysicsWorld* physicsWorld() override;
	virtual void setMajorCharacter(GMCharacter* character) override;

	//renders:
public:
	void drawPolygonFace(GMint polygonFaceNumber);
	void drawMeshFace(GMint meshFaceNumber);
	void drawPatch(GMint patchNumber);
	void drawEntity(GMint leafId);

private:
	void clearBuffer();
	void flushBuffer();
	void updateCamera();
	void calculateVisibleFaces();
	void drawAll();
	void drawSky();
	void drawFaces();
	void draw(GMBSP_Render_BiquadraticPatch& biqp);
	void drawAlwaysVisibleObjects();
	template <typename T> bool setMaterialTexture(T& face, REF Shader& shader);
	void setMaterialLightmap(GMint lightmapid, REF Shader& shader);
	int isClusterVisible(GMint cameraCluster, GMint testCluster);

	//imports:
private:
	void importBSP();
	void initModels();
	void initShaders();
	void initTextures();
	bool findTexture(const char* textureFilename, OUT Image** img);
	void initLightmaps();
	void prepareFaces();
	void preparePolygonFace(GMint polygonFaceNumber, GMint drawSurfaceIndex);
	void prepareMeshFace(GMint meshFaceNumber, GMint drawSurfaceIndex);
	void preparePatch(GMint patchNumber, GMint drawSurfaceIndex);
	void prepareEntities();
	void createEntity(GMBSPEntity* entity);
	GMint calculateLeafNode(const linear_math::Vector3& position);

	// this is usually used by BSPShaderLoader, BSPGameWorldEntityReader, physics world
public:
	BSPData& bspData();
	GMBSPRenderData& renderData();
};

END_NS
#endif