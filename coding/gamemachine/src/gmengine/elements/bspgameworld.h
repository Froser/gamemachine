#ifndef __BSPGAMEWORLD_H__
#define __BSPGAMEWORLD_H__
#include "common.h"
#include "gameworld.h"
#include "bspgameworldprivate.h"
#include "gmengine/controllers/resource_container.h"
BEGIN_NS

class BSPGameWorld : public GameWorld
{
	DEFINE_PRIVATE(BSPGameWorld);
public:
	BSPGameWorld(GamePackage* pk);

public:
	void loadBSP(const char* mapName);
	void setSky(AUTORELEASE GameObject* sky);
	GameObject* getSky();
	void appendObjectAndInit(AUTORELEASE GameObject* obj, bool alwaysVisible);
	using GameWorld::appendObjectAndInit;

public:
	virtual void renderGameWorld() override;
	virtual PhysicsWorld* physicsWorld() override;
	virtual void setMajorCharacter(Character* character) override;

	//renders:
private:
	void updateCamera();
	void calculateVisibleFaces();
	void drawAll();
	void drawSky();
	void drawFaces();
	void drawFace(GMint idx);
	void drawPolygonFace(GMint polygonFaceNumber);
	void drawMeshFace(GMint meshFaceNumber);
	void drawPatch(GMint patchNumber);
	void draw(BSP_Render_BiquadraticPatch& biqp);
	void drawEntity(GMint leafId);
	void drawAlwaysVisibleObjects();
	template <typename T> bool setMaterialTexture(T face, REF Shader& shader);
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
	void preparePolygonFace(int polygonFaceNumber);
	void prepareMeshFace(int meshFaceNumber);
	void preparePatch(int patchNumber);
	void prepareEntities();
	void createEntity(BSPEntity* entity);
	GMint calculateLeafNode(const vmath::vec3& position);

	// this is usually used by BSPShaderLoader, BSPGameWorldEntityReader, physics world
public:
	BSPData& bspData();
	BSPRenderData& renderData();
};

END_NS
#endif