#include "stdafx.h"
#include "gmbspgameworld.h"
#include "gmengine/gmassets.h"
#include "gmdata/imagereader/gmimagereader.h"
#include "foundation/utilities/tools.h"
#include "gmdata/gmimagebuffer.h"
#include "extensions/bsp/data/gmbsp_shader_loader.h"
#include "gmdata/gamepackage/gmgamepackage.h"
#include <algorithm>
#include "gmdata/modelreader/gmmodelreader.h"
#include "foundation/gamemachine.h"
#include "foundation/gmprofile.h"
#include "foundation/gmthread.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "extensions/bsp/physics/gmbspphysicsworld.h"

BEGIN_NS

// 天空
namespace
{
	GMVec2 texcoord[24] = {
		GMVec2(0, 0),
		GMVec2(0, 1),
		GMVec2(1, 1),
		GMVec2(1, 0),

		GMVec2(0, 0),
		GMVec2(0, 1),
		GMVec2(1, 1),
		GMVec2(1, 0),

		GMVec2(0, 0),
		GMVec2(0, 1),
		GMVec2(1, 1),
		GMVec2(1, 0),

		GMVec2(0, 0),
		GMVec2(0, 1),
		GMVec2(1, 1),
		GMVec2(1, 0),

		GMVec2(0, 0),
		GMVec2(0, 1),
		GMVec2(1, 1),
		GMVec2(1, 0),

		GMVec2(0, 0),
		GMVec2(0, 1),
		GMVec2(1, 1),
		GMVec2(1, 0),
	};
}

GMBSPSkyGameObject::GMBSPSkyGameObject(const GMShader& shader, const GMVec3& min, const GMVec3& max)
{
	D(d);
	d->shader = shader;
	d->min = min;
	d->max = max;

	GMModel* obj = nullptr;
	createSkyBox(&obj);
	setAsset(GMAsset(GMAssetType::Model, obj));
}

void GMBSPSkyGameObject::createSkyBox(OUT GMModel** obj)
{
	D(d);
	GMFloat4 f4_min, f4_max;
	d->min.loadFloat4(f4_min);
	d->max.loadFloat4(f4_max);
	GMVec3 vertices[] = {
		//Front
		GMVec3(f4_min[0], f4_max[1], f4_max[2]),
		GMVec3(f4_min[0], f4_min[1], f4_max[2]),
		GMVec3(f4_max[0], f4_max[1], f4_max[2]),
		GMVec3(f4_max[0], f4_min[1], f4_max[2]),

		//Back
		GMVec3(f4_min[0], f4_max[1], f4_min[2]),
		GMVec3(f4_min[0], f4_min[1], f4_min[2]),
		GMVec3(f4_max[0], f4_max[1], f4_min[2]),
		GMVec3(f4_max[0], f4_min[1], f4_min[2]),

		//Left
		GMVec3(f4_min[0], f4_max[1], f4_min[2]),
		GMVec3(f4_min[0], f4_max[1], f4_max[2]),
		GMVec3(f4_min[0], f4_min[1], f4_min[2]),
		GMVec3(f4_min[0], f4_min[1], f4_max[2]),

		//Right
		GMVec3(f4_max[0], f4_max[1], f4_min[2]),
		GMVec3(f4_max[0], f4_max[1], f4_max[2]),
		GMVec3(f4_max[0], f4_min[1], f4_min[2]),
		GMVec3(f4_max[0], f4_min[1], f4_max[2]),

		//Up
		GMVec3(f4_min[0], f4_max[1], f4_min[2]),
		GMVec3(f4_min[0], f4_max[1], f4_max[2]),
		GMVec3(f4_max[0], f4_max[1], f4_min[2]),
		GMVec3(f4_max[0], f4_max[1], f4_max[2]),

		//Down
		GMVec3(f4_min[0], f4_min[1], f4_min[2]),
		GMVec3(f4_min[0], f4_min[1], f4_max[2]),
		GMVec3(f4_max[0], f4_min[1], f4_min[2]),
		GMVec3(f4_max[0], f4_min[1], f4_max[2]),
	};

	// Scaling surface
	const GMint32 SCALING = 2;
	GMVec3 center = (d->min + d->max) / 2.f;
	GMMat4 transScale = Scale(GMVec3(SCALING, 1, SCALING));
	for (GMuint32 i = 0; i < 24; i++)
	{
		GMMat4 transRestore = Translate(center);
		GMMat4 transMoveToAxisOrigin = Translate(-center);
		GMMat4 transFinal = transMoveToAxisOrigin* transScale * transRestore;

		GMVec4 pt = GMVec4(vertices[i], 1) * transFinal;
		vertices[i] = GMVec3(pt);
	}

	GMModel* model = new GMModel();
	*obj = model;
	model->setShader(d->shader);

	GMMesh* mesh = new GMMesh(model);
	for (GMuint32 i = 0; i < 6; i++)
	{
		GMVertex P0 = {
			{ vertices[i * 4].getX(), vertices[i * 4].getY(), vertices[i * 4].getZ() },
			{ 0, 0, 0 },
			{ texcoord[i * 4].getX(), texcoord[i * 4].getY() },
		};
		GMVertex P1 = {
			{ vertices[i * 4 + 1].getX(), vertices[i * 4 + 1].getY(), vertices[i * 4 + 1].getZ() },
			{ 0, 0, 0 },
			{ texcoord[i * 4 + 1].getX(), texcoord[i * 4 + 1].getY() },
		};
		GMVertex P2 = {
			{ vertices[i * 4 + 2].getX(), vertices[i * 4 + 2].getY(), vertices[i * 4 + 2].getZ() },
			{ 0, 0, 0 },
			{ texcoord[i * 4 + 2].getX(), texcoord[i * 4 + 2].getY() },
		};
		GMVertex P3 = {
			{ vertices[i * 4 + 3].getX(), vertices[i * 4 + 3].getY(), vertices[i * 4 + 3].getZ() },
			{ 0, 0, 0 },
			{ texcoord[i * 4 + 3].getX(), texcoord[i * 4 + 3].getY() },
		};

		mesh->vertex(P0);
		mesh->vertex(P2);
		mesh->vertex(P1);
		mesh->vertex(P1);
		mesh->vertex(P2);
		mesh->vertex(P3);
	}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
typedef void (GMBSPGameWorld::*drawFaceHandler)(GMint32);

namespace
{
	void drawFacesToRenderList(
		GMBSPGameWorld* world,
		const Vector<GMint32>& indices,
		drawFaceHandler handler,
		GMBSPSurfaceType targetType
		)
	{
		GMBSPRenderData& renderData = world->renderData();
		for (auto i : indices)
		{
			if (renderData.facesToDraw.isSet(i))
			{
				if (renderData.faceDirectory[i].faceType == 0)
					continue;

				if (renderData.faceDirectory[i].faceType == targetType)
					(world->*handler)(renderData.faceDirectory[i].typeFaceNumber);
			}
		}
	}
}

END_NS

GMBSPGameWorld::GMBSPGameWorld(const IRenderContext* context)
	: GMGameWorld(context)
{
	D(d);
	d->physics = new GMBSPPhysicsWorld(this);
	d->debugConfig = GM.getConfigs().getConfig(GMConfigs::Debug).asDebugConfig();

	d->bspRenderConfigWrapper = d->bspRenderConfig.as<GMBSPRenderConfig>();
	d->bspRenderConfigWrapper.set(GMBSPRenderConfigs::CalculateFace_Bool, true);
	d->bspRenderConfigWrapper.set(GMBSPRenderConfigs::DrawSkyOnly_Bool, false);
}

void GMBSPGameWorld::loadBSP(const GMString& mapName)
{
	D(d);
	GMBuffer buffer;
	GM.getGamePackageManager()->readFile(GMPackageIndex::Maps, mapName, &buffer);
	d->bsp.loadBsp(buffer);
	importBSP();
}

void GMBSPGameWorld::setSky(AUTORELEASE GMGameObject* sky)
{
	D(d);
	d->sky = sky;
	addObjectAndInit(sky);
}

GMGameObject* GMBSPGameWorld::getSky()
{
	D(d);
	return d->sky;
}

void GMBSPGameWorld::renderScene()
{
	GM_PROFILE("renderScene");
	prepareAllToRenderList();
	Base::renderScene();
}

void GMBSPGameWorld::addObjectAndInit(AUTORELEASE GMGameObject* obj, bool alwaysVisible)
{
	D(d);
	GMGameWorld::addObjectAndInit(obj);
	if (alwaysVisible)
		d->render.renderData().alwaysVisibleObjects.push_back(obj);
}

void GMBSPGameWorld::setDefaultLights()
{
	IFactory* factory = GM.getFactory();
	IGraphicEngine* engine = getContext()->getEngine();
	engine->removeLights();

	{
		ILight* light = nullptr;
		factory->createLight(GMLightType::Ambient, &light);
		GM_ASSERT(light);
		GMfloat lightColor[] = { .9f, .9f, .9f };
		light->setLightColor(lightColor);
		engine->addLight(light);
	}

	{
		ILight* light = nullptr;
		factory->createLight(GMLightType::Direct, &light);
		GM_ASSERT(light);
		GMfloat lightColor[] = { 1, 1, 1 };
		light->setLightColor(lightColor);
		engine->addLight(light);
	}
}

void GMBSPGameWorld::setSprite(GMSpriteGameObject* sprite)
{
	D(d);
	d->sprite = sprite;
}

GMSpriteGameObject* GMBSPGameWorld::getSprite()
{
	D(d);
	return d->sprite;
}

void GMBSPGameWorld::setRenderConfig(gm::GMBSPRenderConfigs config, const GMVariant& value)
{
	D(d);
	d->bspRenderConfigWrapper.set(config, value);
}

const GMVariant& GMBSPGameWorld::getRenderConfig(gm::GMBSPRenderConfigs config)
{
	D(d);
	return d->bspRenderConfigWrapper.get(config);
}

Map<GMint32, Set<GMBSPEntity*> >& GMBSPGameWorld::getEntities()
{
	D(d);
	return d->entities;
}

void GMBSPGameWorld::calculateVisibleFaces()
{
	static GMFrustumPlanes frustumPlanes;

	GM_PROFILE("calculateVisibleFaces");
	D(d);
	GMBSPRenderData& rd = d->render.renderData();

	GMCamera& camera = getContext()->getEngine()->getCamera();
	GMVec3 pos = camera.getLookAt().position;
	BSPData& bsp = d->bsp.bspData();

	rd.facesToDraw.clearAll();
	rd.entitiesToDraw.clearAll();
	GMint32 cameraLeaf = calculateLeafNode(pos);
	GMint32 cameraCluster = bsp.leafs[cameraLeaf].cluster;
	camera.getPlanes(frustumPlanes);

	for (GMint32 i = 0; i < bsp.numleafs; ++i)
	{
		//if the leaf is not in the PVS, continue
		if (!isClusterVisible(cameraCluster, bsp.leafs[i].cluster))
			continue;

		//if this leaf does not lie in the frustum, continue
		if (!GMCamera::isBoundingBoxInside(frustumPlanes, rd.leafs[i].boundingBoxVertices))
			continue;

		//loop through faces in this leaf and mark them to be drawn
		for (GMint32 j = 0; j < bsp.leafs[i].numLeafSurfaces; ++j)
		{
			rd.facesToDraw.set(bsp.leafsurfaces[bsp.leafs[i].firstLeafSurface + j]);
		}

		rd.entitiesToDraw.set(i);
	}
}

GMint32 GMBSPGameWorld::calculateLeafNode(const GMVec3& position)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();

	GMint32 currentNode = 0;

	//loop until we find a negative index
	while (currentNode >= 0)
	{
		//if the camera is in front of the plane for this node, assign i to be the front node
		if (bsp.planes[bsp.nodes[currentNode].planeNum].classifyPoint(position) == POINT_IN_FRONT_OF_PLANE)
			currentNode = bsp.nodes[currentNode].children[0]; //front
		else
			currentNode = bsp.nodes[currentNode].children[1]; //back
	}

	//return leaf index
	return ~currentNode;
}

GMint32 GMBSPGameWorld::isClusterVisible(GMint32 cameraCluster, GMint32 testCluster)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	GMint32 index = cameraCluster * rd.visibilityData.bytesPerCluster + testCluster / 8;
	return rd.visibilityData.bitset[index] & (1 << (testCluster & 7));
}

// drawAll将所要需要绘制的对象放入列表
void GMBSPGameWorld::prepareAllToRenderList()
{
	D(d);
	GM_PROFILE("drawAll");
	clearRenderList();
	prepareSkyToRenderList();
	if (!d->bspRenderConfigWrapper.get(GMBSPRenderConfigs::DrawSkyOnly_Bool).toBool())
	{
		if (d->bspRenderConfigWrapper.get(GMBSPRenderConfigs::CalculateFace_Bool).toBool())
			calculateVisibleFaces();
		prepareFacesToRenderList();
	}
	prepareAlwaysVisibleObjects();
}

void GMBSPGameWorld::prepareSkyToRenderList()
{
	D(d);
	addToRenderList(d->sky);
}

void GMBSPGameWorld::prepareFacesToRenderList()
{
	GM_PROFILE("drawFaces");
	::drawFacesToRenderList(this, renderData().polygonIndices, &GMBSPGameWorld::preparePolygonFaceToRenderList, MST_PLANAR);
	::drawFacesToRenderList(this, renderData().meshFaceIndices, &GMBSPGameWorld::prepareMeshFaceToRenderList, MST_TRIANGLE_SOUP);
	::drawFacesToRenderList(this, renderData().patchIndices, &GMBSPGameWorld::preparePatchToRenderList, MST_PATCH);
}

void GMBSPGameWorld::preparePolygonFace(GMint32 polygonFaceNumber, GMint32 drawSurfaceIndex)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();
	rd.polygonIndices.push_back(drawSurfaceIndex);

	GMBSP_Render_Face& polygonFace = rd.polygonFaces[polygonFaceNumber];
	GMGameObject* obj = nullptr;
	GM_ASSERT(rd.polygonFaceObjects.find(&polygonFace) == rd.polygonFaceObjects.end());

	GMShader shader;
	if (!setMaterialTexture(polygonFace, shader))
	{
		gm_warning(gm_dbg_wrap("polygon: {0} texture missing."), GMString(polygonFaceNumber));
		return;
	}
	setMaterialLightmap(polygonFace.lightmapIndex, shader);

	GMModel* model = nullptr;
	d->render.createObject(polygonFace, shader, &model);
	GM_ASSERT(model);
	GMAsset asset = getAssets().addAsset(GMAsset(GMAssetType::Model, model));
	obj = new GMGameObject(asset);

	rd.polygonFaceObjects[&polygonFace] = obj;
	addObjectAndInit(obj);
}

void GMBSPGameWorld::prepareMeshFace(GMint32 meshFaceNumber, GMint32 drawSurfaceIndex)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();
	rd.meshFaceIndices.push_back(drawSurfaceIndex);

	GMBSP_Render_Face& meshFace = rd.meshFaces[meshFaceNumber];
	GMGameObject* obj = nullptr;

	GM_ASSERT(rd.meshFaceObjects.find(&meshFace) == rd.meshFaceObjects.end());
	GMShader shader;
	if (!setMaterialTexture(meshFace, shader))
	{
		gm_warning(gm_dbg_wrap("mesh: {0} texture missing."), GMString(meshFaceNumber));
		return;
	}
	setMaterialLightmap(meshFace.lightmapIndex, shader);

	GMModel* model = nullptr;
	d->render.createObject(meshFace, shader, &model);
	GM_ASSERT(model);

	GMAsset asset = getAssets().addAsset(GMAsset(GMAssetType::Model, model));
	obj = new GMGameObject(asset);
	rd.meshFaceObjects[&meshFace] = obj;
	addObjectAndInit(obj);
}

void GMBSPGameWorld::preparePatch(GMint32 patchNumber, GMint32 drawSurfaceIndex)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();
	rd.patchIndices.push_back(drawSurfaceIndex);

	GMShader shader;
	if (!setMaterialTexture(rd.patches[patchNumber], shader))
	{
		gm_warning(gm_dbg_wrap("patch: {0} texture missing."), GMString(patchNumber));
		return;
	}
	setMaterialLightmap(rd.patches[patchNumber].lightmapIndex, shader);

	for (GMint32 i = 0; i < rd.patches[patchNumber].numQuadraticPatches; ++i)
	{
		GMBSP_Render_BiquadraticPatch& biqp = rd.patches[patchNumber].quadraticPatches[i];
		GM_ASSERT(rd.biquadraticPatchObjects.find(&biqp) == rd.biquadraticPatchObjects.end());

		GMModels* models = nullptr;
		d->render.createObject(biqp, shader, &models);
		GMAsset asset = getAssets().addAsset(GMAsset(GMAssetType::Models, models));
		GMGameObject* obj = new GMGameObject(asset);
		rd.biquadraticPatchObjects[&biqp] = obj;
		addObjectAndInit(obj);
	}
}

void GMBSPGameWorld::preparePolygonFaceToRenderList(GMint32 polygonFaceNumber)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	GMBSP_Render_Face& polygonFace = rd.polygonFaces[polygonFaceNumber];
	GMGameObject* obj = nullptr;
	auto findResult = rd.polygonFaceObjects.find(&polygonFace);
	if (findResult != rd.polygonFaceObjects.end())
		obj = (*findResult).second;
	else
		return;

	GM_ASSERT(obj);
	addToRenderList(obj);
}

void GMBSPGameWorld::prepareMeshFaceToRenderList(GMint32 meshFaceNumber)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	GMBSP_Render_Face& meshFace = rd.meshFaces[meshFaceNumber];
	GMGameObject* obj = nullptr;
	auto findResult = rd.meshFaceObjects.find(&meshFace);
	if (findResult != rd.meshFaceObjects.end())
		obj = (*findResult).second;
	else
		return;

	GM_ASSERT(obj);
	addToRenderList(obj);
}

void GMBSPGameWorld::preparePatchToRenderList(GMint32 patchNumber)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	for (GMint32 i = 0; i < rd.patches[patchNumber].numQuadraticPatches; ++i)
		prepareToRenderList(rd.patches[patchNumber].quadraticPatches[i]);
}

void GMBSPGameWorld::prepareToRenderList(GMBSP_Render_BiquadraticPatch& biqp)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();

	GMGameObject* obj = nullptr;
	auto findResult = rd.biquadraticPatchObjects.find(&biqp);
	if (findResult != rd.biquadraticPatchObjects.end())
		obj = (*findResult).second;
	else
		return;

	GM_ASSERT(obj);
	addToRenderList(obj);
}

void GMBSPGameWorld::prepareAlwaysVisibleObjects()
{
	D(d);
	AlignedVector<GMGameObject*>& objs = d->render.renderData().alwaysVisibleObjects;
	for (auto obj : objs)
	{
		addToRenderList(obj);
	}
}

template <typename T>
bool GMBSPGameWorld::setMaterialTexture(T& face, REF GMShader& shader)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMint32 textureid = face.textureIndex;
	GMint32 lightmapid = face.lightmapIndex;
	const GMString& name = bsp.shaders[textureid].shader;

	// 先从地图Shaders中找，如果找不到，就直接读取材质
	if (!d->shaderLoader.findItem(name, lightmapid, &shader))
	{
		GMAsset asset = getAssets().getAsset(GM_ASSET_TEXTURES + bsp.shaders[textureid].shader);
		if (asset.isEmpty())
			return false;

		shader.getMaterial().ks = shader.getMaterial().kd = GMVec3(0);
		shader.getMaterial().ka = GMVec3(1);
		shader.getTextureList().getTextureSampler(GMTextureType::Ambient).addFrame(asset);
	}
	return true;
}

void GMBSPGameWorld::setMaterialLightmap(GMint32 lightmapid, REF GMShader& shader)
{
	D(d);
	const GMint32 WHITE_LIGHTMAP = -1;
	GMint32 id = 0;

	if (shader.getSurfaceFlag() & SURF_NOLIGHTMAP)
		id = WHITE_LIGHTMAP;
	else
		id = lightmapid >= 0 ? lightmapid : WHITE_LIGHTMAP;

	GMAsset texture = getAssets().getAsset(GM_ASSET_LIGHTMAPS + std::to_string(id));
	shader.getTextureList().getTextureSampler(GMTextureType::Lightmap).addFrame(texture);
}

void GMBSPGameWorld::importBSP()
{
	D(d);
	d->render.generateRenderData(&d->bsp.bspData());
	initShaders();
	initLightmaps();
	initTextures();
	prepareFaces();
	prepareEntities();
	d->physics->initBSPPhysicsWorld();
}

void GMBSPGameWorld::initShaders()
{
	D(d);
	GMString texShadersPath = GM.getGamePackageManager()->pathOf(GMPackageIndex::TexShaders, "");
	d->shaderLoader.init(texShadersPath, this, &d->render.renderData());
	d->shaderLoader.load();
}

void GMBSPGameWorld::initTextures()
{
	D(d);
	D_BASE(db, GMGameWorld);
	BSPData& bsp = d->bsp.bspData();

	IFactory* factory = GM.getFactory();

	for (GMint32 i = 0; i < bsp.numShaders; i++)
	{
		GMBSPShader& shader = bsp.shaders[i];
		// 如果一个texture在shader中已经定义，那么不读取它了，而使用shader中的材质
		if (d->shaderLoader.findItem(shader.shader, 0, nullptr))
			continue;

		GMImage* tex = nullptr;
		if (findTexture(shader.shader, &tex))
		{
			GMTextureAsset texture;
			factory->createTexture(getContext(), tex, texture);
			GM_delete(tex);
			getAssets().addAsset(GM_ASSET_TEXTURES + GMString(shader.shader), texture);
		}
		else
		{
			gm_warning(gm_dbg_wrap("Cannot find texture {0}"), shader.shader);
		}
	}
}

bool GMBSPGameWorld::findTexture(const GMString& textureFilename, OUT GMImage** img)
{
	const GMint32 maxChars = 128;
	static GMString priorities[maxChars] =
	{
		".jpg",
		".tga",
		".png",
		".bmp"
	};
	static GMint32 dem = 4;
	GMGamePackage* pk = GM.getGamePackageManager();

	for (GMint32 i = 0; i < dem; i++)
	{
		GMString fn = textureFilename + priorities[i];
		GMBuffer buf;
		if (!pk->readFile(GMPackageIndex::Textures, fn, &buf))
			continue;

		if (GMImageReader::load(buf.buffer, buf.size, img))
		{
			gm_info(gm_dbg_wrap("loaded texture {0}"), fn);
			return true;
		}
	}
	return false;
}

void GMBSPGameWorld::initLightmaps()
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	IFactory* factory = GM.getFactory();

	const GMint32 BSP_LIGHTMAP_EXT = 128;
	const GMint32 BSP_LIGHTMAP_SIZE = BSP_LIGHTMAP_EXT * BSP_LIGHTMAP_EXT * 3 * sizeof(GMbyte);
	GMint32 numLightmaps = bsp.numLightBytes / (BSP_LIGHTMAP_SIZE * sizeof(GMbyte));

	for (GMint32 i = 0; i < numLightmaps; i++)
	{
		GMbyte* lightmapBytes = bsp.lightBytes.data() + i * BSP_LIGHTMAP_SIZE;
		GMImageBuffer* imgBuf = new GMImageBuffer(GMImageFormat::RGB, BSP_LIGHTMAP_EXT, BSP_LIGHTMAP_EXT, BSP_LIGHTMAP_SIZE, lightmapBytes);
		GMTextureAsset texture;
		factory->createTexture(getContext(), imgBuf, texture);
		GM_delete(imgBuf);
		getAssets().addAsset(GM_ASSET_LIGHTMAPS + GMString(i), texture);
	}

	{
		// Create a white lightmap id = -1
		GMbyte white[3] = { 0xff, 0xff, 0xff };
		GMImageBuffer* whiteBuf = new GMImageBuffer(GMImageFormat::RGB, 1, 1, 3 * sizeof(GMbyte), white);
		GMTextureAsset texture;
		factory->createTexture(getContext(), whiteBuf, texture);
		GM_delete(whiteBuf);
		getAssets().addAsset(GM_ASSET_LIGHTMAPS + GMString(L"-1"), texture);
	}
}

void GMBSPGameWorld::prepareFaces()
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	//loop through faces
	for (GMint32 i = 0; i < bsp.numDrawSurfaces; ++i)
	{
		if (rd.faceDirectory[i].faceType == 0)
			return;

		if (rd.faceDirectory[i].faceType == MST_PLANAR)
			preparePolygonFace(rd.faceDirectory[i].typeFaceNumber, i);

		if (rd.faceDirectory[i].faceType == MST_TRIANGLE_SOUP)
			prepareMeshFace(rd.faceDirectory[i].typeFaceNumber, i);

		if (rd.faceDirectory[i].faceType == MST_PATCH)
			preparePatch(rd.faceDirectory[i].typeFaceNumber, i);
	}
}

void GMBSPGameWorld::prepareEntities()
{
	D(d);
	BSPData& bsp = d->bsp.bspData();

	for (auto entity : bsp.entities)
	{
		BSPGameWorldEntityReader::import(*entity, this);
		GMint32 leaf = calculateLeafNode(MakeVector3(entity->origin));
		d->entities[leaf].insert(entity);
	}
}

BSPData& GMBSPGameWorld::bspData()
{
	D(d);
	return d->bsp.bspData();
}

GMBSPRenderData& GMBSPGameWorld::renderData()
{
	D(d);
	return d->render.renderData();
}
