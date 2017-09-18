#include "stdafx.h"
#include "gmbspgameworld.h"
#include "gmengine/gmassets.h"
#include "gmdatacore/imagereader/gmimagereader.h"
#include "foundation/utilities/utilities.h"
#include "gmdatacore/imagebuffer.h"
#include "gmdatacore/bsp/gmbsp_shader_loader.h"
#include "gmdatacore/gamepackage/gmgamepackage.h"
#include <algorithm>
#include "gmdatacore/modelreader/gmmodelreader.h"
#include "foundation/gamemachine.h"
#include "foundation/gmprofile.h"
#include "foundation/gmthreads.h"
#include "gmgameobject.h"
#include "gmphysics/bsp/gmbspphysicsworld.h"

BEGIN_NS

typedef void (GMBSPGameWorld::*drawFaceHandler)(GMint);

inline void drawFaces(
	GMBSPGameWorld* world,
	const Vector<GMint>& indices,
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

inline void drawEntities(GMBSPGameWorld* world)
{
	BSPData& bsp = world->bspData();
	GMBSPRenderData& renderData = world->renderData();

	for (GMint i = 0; i < bsp.numleafs; i++)
	{
		if (renderData.entitiesToDraw.isSet(i))
			world->drawEntity(i);
	}
}

END_NS

GMBSPGameWorld::GMBSPGameWorld()
	: GMGameWorld()
{
	D(d);
	d->physics = new GMBSPPhysicsWorld(this);
}

GMBSPGameWorld::~GMBSPGameWorld()
{
	D(d);
	if (d->physics)
		delete d->physics;
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
	appendObjectAndInit(sky);
}

GMGameObject* GMBSPGameWorld::getSky()
{
	D(d);
	return d->sky;
}

void GMBSPGameWorld::renderGameWorld()
{
	GM_PROFILE(renderGameWorld);
	Base::renderGameWorld();
	drawAll();
}

GMPhysicsWorld* GMBSPGameWorld::physicsWorld()
{
	D(d);
	return d->physics;
}

void GMBSPGameWorld::appendObjectAndInit(AUTORELEASE GMGameObject* obj, bool alwaysVisible)
{
	D(d);
	GMGameWorld::appendObjectAndInit(obj);
	if (alwaysVisible)
		d->render.renderData().alwaysVisibleObjects.push_back(obj);
}

Map<GMint, Set<GMBSPEntity*> >& GMBSPGameWorld::getEntities()
{
	D(d);
	return d->entities;
}

void GMBSPGameWorld::calculateVisibleFaces()
{
	GM_PROFILE(calculateVisibleFaces);
	D(d);
	GMBSPRenderData& rd = d->render.renderData();

	GMCamera& camera = GM.getCamera();
	PositionState pos = camera.getPositionState();
	BSPData& bsp = d->bsp.bspData();

	rd.facesToDraw.clearAll();
	rd.entitiesToDraw.clearAll();
	GMint cameraLeaf = calculateLeafNode(pos.position);
	GMint cameraCluster = bsp.leafs[cameraLeaf].cluster;

	for (GMint i = 0; i < bsp.numleafs; ++i)
	{
		//if the leaf is not in the PVS, continue
		if (!isClusterVisible(cameraCluster, bsp.leafs[i].cluster))
			continue;

		//if this leaf does not lie in the frustum, continue
		if (!camera.getFrustum().isBoundingBoxInside(rd.leafs[i].boundingBoxVertices))
			continue;

		//loop through faces in this leaf and mark them to be drawn
		for (GMint j = 0; j < bsp.leafs[i].numLeafSurfaces; ++j)
		{
			rd.facesToDraw.set(bsp.leafsurfaces[bsp.leafs[i].firstLeafSurface + j]);
		}

		rd.entitiesToDraw.set(i);
	}
}

GMint GMBSPGameWorld::calculateLeafNode(const linear_math::Vector3& position)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();

	GMint currentNode = 0;

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

GMint GMBSPGameWorld::isClusterVisible(GMint cameraCluster, GMint testCluster)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	GMint index = cameraCluster * rd.visibilityData.bytesPerCluster + testCluster / 8;
	return rd.visibilityData.bitset[index] & (1 << (testCluster & 7));
}

// drawAll将所要需要绘制的对象放入列表
void GMBSPGameWorld::drawAll()
{
	GM_PROFILE(drawAll);
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->newFrame();
	clearBuffer();
	drawSky();
	if (!GMGetDebugState(DRAW_ONLY_SKY))
	{
		if (GMGetDebugState(CALCULATE_BSP_FACE))
			calculateVisibleFaces();
		drawFaces();
	}
	flushBuffer();

	engine->beginBlend();
	engine->beginFullRendering();
	clearBuffer();
	drawAlwaysVisibleObjects();
	flushBuffer();
	engine->endFullRendering();
	engine->endBlend();
}

void GMBSPGameWorld::drawSky()
{
	D(d);
	if (d->sky)
		d->renderBuffer.push_back(d->sky);
}

void GMBSPGameWorld::drawFaces()
{
	GM_PROFILE(drawFaces);
	::drawFaces(this, renderData().polygonIndices, &GMBSPGameWorld::drawPolygonFace, MST_PLANAR);
	::drawFaces(this, renderData().meshFaceIndices, &GMBSPGameWorld::drawMeshFace, MST_TRIANGLE_SOUP);
	::drawFaces(this, renderData().patchIndices, &GMBSPGameWorld::drawPatch, MST_PATCH);
	::drawEntities(this);
}

void GMBSPGameWorld::clearBuffer()
{
	D(d);
	d->renderBuffer.clear();
}

void GMBSPGameWorld::flushBuffer()
{
	GM_PROFILE(flushBuffer);

	D(d);
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->drawObjects(d->renderBuffer.data(), d->renderBuffer.size());
}

void GMBSPGameWorld::preparePolygonFace(GMint polygonFaceNumber, GMint drawSurfaceIndex)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();
	rd.polygonIndices.push_back(drawSurfaceIndex);

	GMBSP_Render_Face& polygonFace = rd.polygonFaces[polygonFaceNumber];
	GMGameObject* obj = nullptr;
	GM_ASSERT(rd.polygonFaceObjects.find(&polygonFace) == rd.polygonFaceObjects.end());

	Shader shader;
	shader.setFrontFace(GMS_FrontFace::CLOCKWISE);

	if (!setMaterialTexture(polygonFace, shader))
	{
		gm_warning(_L("polygon: %d texture missing."), polygonFaceNumber);
		return;
	}
	setMaterialLightmap(polygonFace.lightmapIndex, shader);

	GMModel* coreObj;
	d->render.createObject(polygonFace, shader, &coreObj);
	obj = new GMGameObject(coreObj);

	rd.polygonFaceObjects[&polygonFace] = obj;
	appendObjectAndInit(obj);
}

void GMBSPGameWorld::prepareMeshFace(GMint meshFaceNumber, GMint drawSurfaceIndex)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();
	rd.meshFaceIndices.push_back(drawSurfaceIndex);

	GMBSP_Render_Face& meshFace = rd.meshFaces[meshFaceNumber];
	GMGameObject* obj = nullptr;

	GM_ASSERT(rd.meshFaceObjects.find(&meshFace) == rd.meshFaceObjects.end());
	Shader shader;
	shader.setFrontFace(GMS_FrontFace::CLOCKWISE);

	if (!setMaterialTexture(meshFace, shader))
	{
		gm_warning(_L("mesh: %d texture missing."), meshFaceNumber);
		return;
	}
	setMaterialLightmap(meshFace.lightmapIndex, shader);

	GMModel* coreObj;
	d->render.createObject(meshFace, shader, &coreObj);
	obj = new GMGameObject(coreObj);
	rd.meshFaceObjects[&meshFace] = obj;
	appendObjectAndInit(obj);
}

void GMBSPGameWorld::preparePatch(GMint patchNumber, GMint drawSurfaceIndex)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();
	rd.patchIndices.push_back(drawSurfaceIndex);

	Shader shader;
	shader.setFrontFace(GMS_FrontFace::CLOCKWISE);

	if (!setMaterialTexture(rd.patches[patchNumber], shader))
	{
		gm_warning(_L("patch: %d texture missing."), patchNumber);
		return;
	}
	setMaterialLightmap(rd.patches[patchNumber].lightmapIndex, shader);

	for (GMint i = 0; i < rd.patches[patchNumber].numQuadraticPatches; ++i)
	{
		GMBSP_Render_BiquadraticPatch& biqp = rd.patches[patchNumber].quadraticPatches[i];
		GM_ASSERT(rd.biquadraticPatchObjects.find(&biqp) == rd.biquadraticPatchObjects.end());

		GMModel* coreObj;
		d->render.createObject(biqp, shader, &coreObj);
		GMGameObject* obj = new GMGameObject(coreObj);
		rd.biquadraticPatchObjects[&biqp] = obj;
		appendObjectAndInit(obj);
	}
}

void GMBSPGameWorld::drawPolygonFace(GMint polygonFaceNumber)
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
	d->renderBuffer.push_back(obj);
}

void GMBSPGameWorld::drawMeshFace(GMint meshFaceNumber)
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
	d->renderBuffer.push_back(obj);
}

void GMBSPGameWorld::drawPatch(GMint patchNumber)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	for (GMint i = 0; i < rd.patches[patchNumber].numQuadraticPatches; ++i)
		draw(rd.patches[patchNumber].quadraticPatches[i]);
}

void GMBSPGameWorld::draw(GMBSP_Render_BiquadraticPatch& biqp)
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
	d->renderBuffer.push_back(obj);
}

void GMBSPGameWorld::drawEntity(GMint leafId)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();

	Set<GMBSPEntity*>& entities = d->entities[leafId];
	std::for_each(entities.begin(), entities.end(), [&rd, &d](GMBSPEntity* e)
	{
		GMGameObject* obj = rd.entitiyObjects[e];
		if (obj)
			d->renderBuffer.push_back(obj);
	});
}

void GMBSPGameWorld::drawAlwaysVisibleObjects()
{
	D(d);
	AlignedVector<GMGameObject*>& objs = d->render.renderData().alwaysVisibleObjects;
	for (auto& obj : objs)
	{
		d->renderBuffer.push_back(obj);
	}
}

template <typename T>
bool GMBSPGameWorld::setMaterialTexture(T& face, REF Shader& shader)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMint textureid = face.textureIndex;
	GMint lightmapid = face.lightmapIndex;
	const GMString& name = bsp.shaders[textureid].shader;

	// 先从地图Shaders中找，如果找不到，就直接读取材质
	if (!d->shaderLoader.findItem(name, lightmapid, &shader))
	{
		GMTextureAssets& tc = getAssets().getTextureContainer();
		const GMTextureAssets::TextureItemType* item = tc.find(bsp.shaders[textureid].shader);
		if (!item)
			return false;
		shader.getTexture().getTextureFrames(GMTextureType::AMBIENT, 0).addFrame(item->texture);
	}
	return true;
}

void GMBSPGameWorld::setMaterialLightmap(GMint lightmapid, REF Shader& shader)
{
	D(d);
	const GMint WHITE_LIGHTMAP = -1;
	GMTextureAssets_ID& tc = getAssets().getLightmapContainer();
	const GMTextureAssets_ID::TextureItemType* item = nullptr;
	if (shader.getSurfaceFlag() & SURF_NOLIGHTMAP)
		item = tc.find(WHITE_LIGHTMAP);
	else
		item = lightmapid >= 0 ? tc.find(lightmapid) : tc.find(WHITE_LIGHTMAP);

	shader.getTexture().getTextureFrames(GMTextureType::LIGHTMAP, 0).addFrame(item->texture);
}

void GMBSPGameWorld::importBSP()
{
	D(d);
	d->render.generateRenderData(&d->bsp.bspData());
	initModels();
	initShaders();
	initLightmaps();
	initTextures();
	prepareFaces();
	prepareEntities();
	d->physics->initBSPPhysicsWorld();
}

void GMBSPGameWorld::initModels()
{
	D(d);
	GMString modelPath = GM.getGamePackageManager()->pathOf(GMPackageIndex::Models, "");
	d->modelLoader.init(modelPath, this);
	d->modelLoader.load();
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

	for (GMint i = 0; i < bsp.numShaders; i++)
	{
		GMBSPShader& shader = bsp.shaders[i];
		// 如果一个texture在shader中已经定义，那么不读取它了，而使用shader中的材质
		if (d->shaderLoader.findItem(shader.shader, 0, nullptr))
			continue;

		GMImage* tex = nullptr;
		if (findTexture(shader.shader, &tex))
		{
			ITexture* texture;
			factory->createTexture(tex, &texture);

			GMTextureAssets::TextureItemType item = { shader.shader, texture };
			getAssets().getTextureContainer().insert(item);
		}
		else
		{
			gm_warning("Cannot find texture %s", shader.shader);
		}
	}
}

bool GMBSPGameWorld::findTexture(const GMString& textureFilename, OUT GMImage** img)
{
	const GMint maxChars = 128;
	static GMString priorities[maxChars] =
	{
		".jpg",
		".tga",
		".png",
		".bmp"
	};
	static GMint dem = 4;
	GMGamePackage* pk = GM.getGamePackageManager();

	for (GMint i = 0; i < dem; i++)
	{
		GMString fn = textureFilename + priorities[i];
		GMBuffer buf;
		if (!pk->readFile(GMPackageIndex::Textures, fn, &buf))
			continue;

		if (GMImageReader::load(buf.buffer, buf.size, img))
		{
			gm_info(_L("loaded texture %Ls"), fn.toStdWString().c_str());
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

	const GMint BSP_LIGHTMAP_EXT = 128;
	const GMint BSP_LIGHTMAP_SIZE = BSP_LIGHTMAP_EXT * BSP_LIGHTMAP_EXT * 3 * sizeof(GMbyte);
	GMint numLightmaps = bsp.numLightBytes / (BSP_LIGHTMAP_SIZE * sizeof(GMbyte));

	for (GMint i = 0; i < numLightmaps; i++)
	{
		GMbyte* lightmapBytes = bsp.lightBytes.data() + i * BSP_LIGHTMAP_SIZE;
		ImageBuffer* imgBuf = new ImageBuffer(BSP_LIGHTMAP_EXT, BSP_LIGHTMAP_EXT, BSP_LIGHTMAP_SIZE, lightmapBytes);
		ITexture* texture = nullptr;
		factory->createTexture(imgBuf, &texture);

		GMTextureAssets_ID::TextureItemType item = { i, texture };
		getAssets().getLightmapContainer().insert(item);
	}

	{
		// Create a white lightmap id = -1
		GMbyte white[3] = { 0xff, 0xff, 0xff };
		ImageBuffer* whiteBuf = new ImageBuffer(1, 1, 3 * sizeof(GMbyte), white);
		ITexture* texture = nullptr;
		factory->createTexture(whiteBuf, &texture);

		GMTextureAssets_ID::TextureItemType item = { -1, texture };
		getAssets().getLightmapContainer().insert(item);
	}
}

void GMBSPGameWorld::prepareFaces()
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	//loop through faces
	for (GMint i = 0; i < bsp.numDrawSurfaces; ++i)
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

	for (auto& entity : bsp.entities)
	{
		BSPGameWorldEntityReader::import(*entity, this);
		GMint leaf = calculateLeafNode(entity->origin);
		d->entities[leaf].insert(entity);
		createEntity(entity);
	}
}

void GMBSPGameWorld::createEntity(GMBSPEntity* entity)
{
	D(d);
	D_BASE(db, GMGameWorld);

	GMBSPEPair* p = entity->epairs;
	GMString classname;
	while (p)
	{
		if (p->key == "classname")
			classname = p->value;
		p = p->next;
	}

	Model* m = d->modelLoader.find(classname);
	if (!m)
	{
		gm_info("model '%s' is not defined in model list, skipped.", classname.toStdString().c_str());
		return;
	}

	// 不创建这个实体
	if (!m->create)
		return;

	GMBSPRenderData& rd = d->render.renderData();

	GM_ASSERT(rd.entitiyObjects.find(entity) == rd.entitiyObjects.end());
	GMAssets& rc = getAssets();
	GMEntityObject* entityObject = nullptr;
	GMModel* model = nullptr;
	if (!strlen(m->model))
	{
		// 如果没有指定model，先创建一个默认的立方体model吧
		Shader shader;
		//if (!setMaterialTexture(meshFace, shader))
		//{
		//	gm_warning("mesh: %d texture missing.", meshFaceNumber);
		//	return;
		//}
		//setMaterialLightmap(meshFace.lightmapIndex, shader);
		gm_warning("No model selected. Create a default cube instead.");
		d->render.createBox(m->extents, entity->origin, shader, &model);
		GMModelContainerItemIndex index = rc.getModelContainer().insert(model);
		entityObject = new GMEntityObject(*this, index);
	}
	else
	{
		GMBuffer buf;
		GMString fn(m->model);
		fn.append("/");
		fn.append(m->model);
		fn.append(".obj");

		GMModelContainerItemIndex index;
		auto iter = d->entitiesCache.find(fn);
		if (iter != d->entitiesCache.end())
		{
			index = iter->second;
		}
		else
		{
			GMGamePackage& pk = *GM.getGamePackageManager();
			GMString path = pk.pathOf(GMPackageIndex::Models, fn);
			GMModelLoadSettings settings = {
				pk,
				path,
				m->model
			};

			if (!GMModelReader::load(settings, &model))
			{
				gm_warning(_L("parse model file failed."));
				return;
			}
			index = rc.getModelContainer().insert(model);
			d->entitiesCache[fn] = index;
		}
		entityObject = new GMEntityObject(*this, index);
		entityObject->setTranslate(linear_math::translate(entity->origin));
		entityObject->setScaling(linear_math::scale(m->extents[0], m->extents[1], m->extents[2]));
	}

	GM_ASSERT(entityObject);
	rd.entitiyObjects[entity] = entityObject;
	appendObjectAndInit(entityObject);
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
