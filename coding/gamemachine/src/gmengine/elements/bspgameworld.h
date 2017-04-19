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
	void drawPolygonFace(int polygonFaceNumber);
	void drawMeshFace(int meshFaceNumber);
	void drawPatch(int patchNumber);
	void draw(BSP_Render_BiquadraticPatch& biqp);
	template <typename T> bool setMaterialTexture(T face, REF Shader& shader);
	void setMaterialLightmap(GMint lightmapid, REF Shader& shader);
	int isClusterVisible(int cameraCluster, int testCluster);

	//imports:
private:
	void importBSP();
	void initShaders();
	void initTextures();
	bool findTexture(const char* textureFilename, OUT Image** img);
	void initLightmaps();
	void importEntities();
	void prepareFaces();
	void preparePolygonFace(int polygonFaceNumber);
	void prepareMeshFace(int meshFaceNumber);
	void preparePatch(int patchNumber);
	GMint calculateLeafNode(const vmath::vec3& position);

	// this is usually used by BSPShaderLoader, BSPGameWorldEntityReader, physics world
public:
	const char* bspWorkingDirectory();
	BSPData& bspData();
	BSPRenderData& renderData();
};

END_NS
#endif