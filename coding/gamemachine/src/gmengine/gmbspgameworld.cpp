#include "stdafx.h"
#include "gmbspgameworld.h"
#include "gmcharacter.h"
#include "gmengine/controllers/resource_container.h"
#include "gmdatacore/imagereader/imagereader.h"
#include "foundation/utilities/utilities.h"
#include "gmdatacore/imagebuffer.h"
#include "gmdatacore/bsp/bsp_shader_loader.h"
#include "gmdatacore/gamepackage.h"
#include <algorithm>
#include "gmdatacore/modelreader/gmmodelreader.h"
#include "foundation/gamemachine.h"

GMBSPGameWorld::GMBSPGameWorld(GamePackage* pk)
	: GMGameWorld(pk)
{
	D(d);
	d->physics.reset(new GMBSPPhysicsWorld(this));
}

void GMBSPGameWorld::loadBSP(const char* mapName)
{
	D(d);
	D_BASE(GMGameWorld, db);
	GamePackageBuffer buffer;
	db->gamePackage->readFile(PI_MAPS, mapName, &buffer);
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

void GMBSPGameWorld::updateCamera()
{
	D(d);

	GMCharacter* character = getMajorCharacter();
	character->updateCamera();
	CameraLookAt& lookAt = character->getLookAt();
	GameMachine::instance().getGraphicEngine()->updateCameraView(lookAt);
}

void GMBSPGameWorld::renderGameWorld()
{
	D(d);
	GameMachine::instance().getGraphicEngine()->newFrame();
	updateCamera();
	drawAll();
}

GMPhysicsWorld* GMBSPGameWorld::physicsWorld()
{
	D(d);
	return d->physics;
}

void GMBSPGameWorld::setMajorCharacter(GMCharacter* character)
{
	D(d);
	d->physics->setCamera(character);
	GMGameWorld::setMajorCharacter(character);
}

void GMBSPGameWorld::appendObjectAndInit(AUTORELEASE GMGameObject* obj, bool alwaysVisible)
{
	D(d);
	GMGameWorld::appendObjectAndInit(obj);
	if (alwaysVisible)
		d->render.renderData().alwaysVisibleObjects.push_back(obj);
}

std::map<GMint, std::set<BSPEntity*> >& GMBSPGameWorld::getEntities()
{
	D(d);
	return d->entities;
}

void GMBSPGameWorld::calculateVisibleFaces()
{
	D(d);
	D_BASE(GMGameWorld, dbase);
	GMBSPRenderData& rd = d->render.renderData();

	GMCharacter* character = getMajorCharacter();
	PositionState pos = character->getPositionState();
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
		GMCharacter* character = getMajorCharacter();
		if (!character->getFrustum().isBoundingBoxInside(rd.leafs[i].boundingBoxVertices))
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
	D(d);
	drawSky();
	if (!DBG_INT(DRAW_ONLY_SKY))
	{
		if (DBG_INT(CALCULATE_BSP_FACE))
			calculateVisibleFaces();
		drawFaces();
	}
	
	drawAlwaysVisibleObjects();
}

void GMBSPGameWorld::drawSky()
{
	D(d);
	if (d->sky)
		GameMachine::instance().getGraphicEngine()->drawObject(d->sky);
}

void GMBSPGameWorld::drawFaces()
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	//loop through faces
	for (GMint i = 0; i < bsp.numDrawSurfaces; ++i)
	{
		//if this face is to be drawn, draw it
		if (rd.facesToDraw.isSet(i))
			drawFace(i);
	}

	for (GMint i = 0; i < bsp.numleafs; i++)
	{
		if (rd.entitiesToDraw.isSet(i))
			drawEntity(i);
	}
}

void GMBSPGameWorld::drawFace(GMint idx)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	//look this face up in the face directory
	if (rd.faceDirectory[idx].faceType == 0)
		return;

	if (rd.faceDirectory[idx].faceType == MST_PLANAR)
		drawPolygonFace(rd.faceDirectory[idx].typeFaceNumber);

	if (rd.faceDirectory[idx].faceType == MST_TRIANGLE_SOUP)
		drawMeshFace(rd.faceDirectory[idx].typeFaceNumber);
	
	if (rd.faceDirectory[idx].faceType == MST_PATCH)
		drawPatch(rd.faceDirectory[idx].typeFaceNumber);
}

void GMBSPGameWorld::preparePolygonFace(GMint polygonFaceNumber)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();

	BSP_Render_Face& polygonFace = rd.polygonFaces[polygonFaceNumber];
	GMGameObject* obj = nullptr;
	ASSERT(rd.polygonFaceObjects.find(&polygonFace) == rd.polygonFaceObjects.end());

	Shader shader;
	if (!setMaterialTexture(polygonFace, shader))
	{
		gm_warning("polygon: %d texture missing.", polygonFaceNumber);
		return;
	}
	setMaterialLightmap(polygonFace.lightmapIndex, shader);

	Object* coreObj;
	d->render.createObject(polygonFace, shader, &coreObj);
	obj = new GMGameObject(coreObj);

	rd.polygonFaceObjects[&polygonFace] = obj;
	appendObjectAndInit(obj);
}

void GMBSPGameWorld::prepareMeshFace(GMint meshFaceNumber)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();

	BSP_Render_Face& meshFace = rd.meshFaces[meshFaceNumber];
	GMGameObject* obj = nullptr;

	ASSERT(rd.meshFaceObjects.find(&meshFace) == rd.meshFaceObjects.end());
	Shader shader;
	if (!setMaterialTexture(meshFace, shader))
	{
		gm_warning("mesh: %d texture missing.", meshFaceNumber);
		return;
	}
	setMaterialLightmap(meshFace.lightmapIndex, shader);

	Object* coreObj;
	d->render.createObject(meshFace, shader, &coreObj);
	obj = new GMGameObject(coreObj);
	rd.meshFaceObjects[&meshFace] = obj;
	appendObjectAndInit(obj);
}

void GMBSPGameWorld::preparePatch(GMint patchNumber)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	Shader shader;
	if (!setMaterialTexture(rd.patches[patchNumber], shader))
	{
		gm_warning("patch: %d texture missing.", patchNumber);
		return;
	}
	setMaterialLightmap(rd.patches[patchNumber].lightmapIndex, shader);

	for (GMint i = 0; i < rd.patches[patchNumber].numQuadraticPatches; ++i)
	{
		BSP_Render_BiquadraticPatch& biqp = rd.patches[patchNumber].quadraticPatches[i];
		ASSERT(rd.biquadraticPatchObjects.find(&biqp) == rd.biquadraticPatchObjects.end());

		Object* coreObj;
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

	BSP_Render_Face& polygonFace = rd.polygonFaces[polygonFaceNumber];
	GMGameObject* obj = nullptr;
	auto findResult = rd.polygonFaceObjects.find(&polygonFace);
	if (findResult != rd.polygonFaceObjects.end())
		obj = (*findResult).second;
	else
		return;

	ASSERT(obj);
	
	GameMachine::instance().getGraphicEngine()->drawObject(obj);
}

void GMBSPGameWorld::drawMeshFace(GMint meshFaceNumber)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	BSP_Render_Face& meshFace = rd.meshFaces[meshFaceNumber];
	GMGameObject* obj = nullptr;
	auto findResult = rd.meshFaceObjects.find(&meshFace);
	if (findResult != rd.meshFaceObjects.end())
		obj = (*findResult).second;
	else
		return;

	ASSERT(obj);
	GameMachine::instance().getGraphicEngine()->drawObject(obj);
}

void GMBSPGameWorld::drawPatch(GMint patchNumber)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMBSPRenderData& rd = d->render.renderData();

	for (GMint i = 0; i < rd.patches[patchNumber].numQuadraticPatches; ++i)
		draw(rd.patches[patchNumber].quadraticPatches[i]);
}

void GMBSPGameWorld::draw(BSP_Render_BiquadraticPatch& biqp)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();

	GMGameObject* obj = nullptr;
	auto findResult = rd.biquadraticPatchObjects.find(&biqp);
	if (findResult != rd.biquadraticPatchObjects.end())
		obj = (*findResult).second;
	else
		return;

	ASSERT(obj);
	GameMachine::instance().getGraphicEngine()->drawObject(obj);
}

#ifdef NO_LAMBDA
struct __renderIter
{
	__renderIter(GMBSPRenderData& _rd, GMBSPGameWorld::Data& _d) : rd(_rd), d(_d) {}
	void operator()(BSPEntity* e)
	{
		GMGameObject* obj = rd.entitiyObjects[e];
		if (obj)
			GameMachine::instance().getGraphicEngine()->drawObject(obj);
	}
	
	GMBSPRenderData& rd;
	GMBSPGameWorld::Data d;
};
#endif

void GMBSPGameWorld::drawEntity(GMint leafId)
{
	D(d);
	GMBSPRenderData& rd = d->render.renderData();

	std::set<BSPEntity*>& entities = d->entities[leafId];
#ifdef NO_LAMBDA
	__renderIter func(rd, d);
	std::for_each(entities.begin(), entities.end(), func);
#else
	std::for_each(entities.begin(), entities.end(), [&rd, &d](BSPEntity* e)
	{
		GMGameObject* obj = rd.entitiyObjects[e];
		if (obj)
			GameMachine::instance().getGraphicEngine()->drawObject(obj);
	});
#endif
}

void GMBSPGameWorld::drawAlwaysVisibleObjects()
{
	D(d);
	AlignedVector<GMGameObject*>& objs = d->render.renderData().alwaysVisibleObjects;
	for (auto iter = objs.begin(); iter != objs.end(); iter++)
	{
		GameMachine::instance().getGraphicEngine()->drawObject(*iter);
	}
}

template <typename T>
bool GMBSPGameWorld::setMaterialTexture(T face, REF Shader& shader)
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	GMuint textureid = face.textureIndex;
	GMuint lightmapid = face.lightmapIndex;
	const char* name = bsp.shaders[textureid].shader;

	// 先从地图Shaders中找，如果找不到，就直接读取材质
	if (!d->shaderLoader.findItem(name, lightmapid, &shader))
	{
		ResourceContainer* rc = GameMachine::instance().getGraphicEngine()->getResourceContainer();
		TextureContainer& tc = rc->getTextureContainer();
		const TextureContainer::TextureItemType* item = tc.find(bsp.shaders[textureid].shader);
		if (!item)
			return false;
		shader.texture.textures[TEXTURE_INDEX_AMBIENT].frames[0] = item->texture;
		shader.texture.textures[TEXTURE_INDEX_AMBIENT].frameCount = 1;
	}
	return true;
}

void GMBSPGameWorld::setMaterialLightmap(GMint lightmapid, REF Shader& shader)
{
	D(d);
	const GMint WHITE_LIGHTMAP = -1;
	ResourceContainer* rc = GameMachine::instance().getGraphicEngine()->getResourceContainer();
	TextureContainer_ID& tc = rc->getLightmapContainer();
	const TextureContainer_ID::TextureItemType* item = nullptr;
	if (shader.surfaceFlag & SURF_NOLIGHTMAP)
		item = tc.find(WHITE_LIGHTMAP);
	else
		item = lightmapid >= 0 ? tc.find(lightmapid) : tc.find(WHITE_LIGHTMAP);

	shader.texture.textures[TEXTURE_INDEX_LIGHTMAP].frames[0] = item->texture;
	shader.texture.textures[TEXTURE_INDEX_LIGHTMAP].frameCount = 1;
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
	initialize();
	d->physics->initBSPPhysicsWorld();
}

void GMBSPGameWorld::initModels()
{
	D(d);
	D_BASE(GMGameWorld, db);
	std::string modelPath = db->gamePackage->pathOf(PI_MODELS, "");
	d->modelLoader.init(modelPath.c_str(), this);
	d->modelLoader.load();
}

void GMBSPGameWorld::initShaders()
{
	D(d);
	D_BASE(GMGameWorld, db);
	std::string texShadersPath = db->gamePackage->pathOf(PI_TEXSHADERS, "");
	d->shaderLoader.init(texShadersPath.c_str(), this, &d->render.renderData());
	d->shaderLoader.load();
}

void GMBSPGameWorld::initTextures()
{
	D(d);
	D_BASE(GMGameWorld, db);
	BSPData& bsp = d->bsp.bspData();

	IFactory* factory = GameMachine::instance().getFactory();
	ResourceContainer* rc = GameMachine::instance().getGraphicEngine()->getResourceContainer();

	for (GMint i = 0; i < bsp.numShaders; i++)
	{
		BSPShader& shader = bsp.shaders[i];
		// 如果一个texture在shader中已经定义，那么不读取它了，而使用shader中的材质
		if (d->shaderLoader.findItem(shader.shader, 0, nullptr))
			continue;

		Image* tex = nullptr;
		if (findTexture(shader.shader, &tex))
		{
			ITexture* texture;
			factory->createTexture(tex, &texture);

			TextureContainer::TextureItemType item;
			item.id = shader.shader;
			item.texture = texture;
			rc->getTextureContainer().insert(item);
		}
		else
		{
			gm_warning("Cannot find texture %s", shader.shader);
		}
	}
}

bool GMBSPGameWorld::findTexture(const char* textureFilename, OUT Image** img)
{
	const GMint maxChars = 128;
	static std::string priorities[maxChars] =
	{
		".jpg",
		".tga",
		".png",
		".bmp"
	};
	static GMint dem = 4;
	GamePackage* pk = getGamePackage();

	for (GMint i = 0; i < dem; i++)
	{
		std::string fn = textureFilename + priorities[i];
		GamePackageBuffer buf;
		if (!pk->readFile(PI_TEXTURES, fn.c_str(), &buf))
			continue;

		if (ImageReader::load(buf.buffer, buf.size, img))
		{
			gm_info("loaded texture %s", fn.c_str());
			return true;
		}
	}
	return false;
}

void GMBSPGameWorld::initLightmaps()
{
	D(d);
	BSPData& bsp = d->bsp.bspData();
	IFactory* factory = GameMachine::instance().getFactory();
	ResourceContainer* rc = GameMachine::instance().getGraphicEngine()->getResourceContainer();

	const GMint BSP_LIGHTMAP_EXT = 128;
	const GMint BSP_LIGHTMAP_SIZE = BSP_LIGHTMAP_EXT * BSP_LIGHTMAP_EXT * 3 * sizeof(GMbyte);
	GMint numLightmaps = bsp.numLightBytes / (BSP_LIGHTMAP_SIZE * sizeof(GMbyte));

	for (GMint i = 0; i < numLightmaps; i++)
	{
		GMbyte* lightmapBytes = bsp.lightBytes.data() + i * BSP_LIGHTMAP_SIZE;
		ImageBuffer* imgBuf = new ImageBuffer(BSP_LIGHTMAP_EXT, BSP_LIGHTMAP_EXT, BSP_LIGHTMAP_SIZE, lightmapBytes);
		ITexture* texture = nullptr;
		factory->createTexture(imgBuf, &texture);

		TextureContainer_ID::TextureItemType item;
		item.id = i;
		item.texture = texture;
		rc->getLightmapContainer().insert(item);
	}

	{
		// Create a white lightmap id = -1
		GMbyte white[3] = { 0xff, 0xff, 0xff };
		ImageBuffer* whiteBuf = new ImageBuffer(1, 1, 3 * sizeof(GMbyte), white);
		ITexture* texture = nullptr;
		factory->createTexture(whiteBuf, &texture);

		TextureContainer_ID::TextureItemType item;
		item.id = -1;
		item.texture = texture;
		rc->getLightmapContainer().insert(item);
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
			preparePolygonFace(rd.faceDirectory[i].typeFaceNumber);

		if (rd.faceDirectory[i].faceType == MST_TRIANGLE_SOUP)
			prepareMeshFace(rd.faceDirectory[i].typeFaceNumber);

		if (rd.faceDirectory[i].faceType == MST_PATCH)
			preparePatch(rd.faceDirectory[i].typeFaceNumber);
	}
}

void GMBSPGameWorld::prepareEntities()
{
	D(d);
	BSPData& bsp = d->bsp.bspData();

	for (auto iter = bsp.entities.begin(); iter != bsp.entities.end(); iter++)
	{
		BSPGameWorldEntityReader::import(*iter, this);

		GMint leaf = calculateLeafNode((*iter).origin);
		d->entities[leaf].insert(&(*iter));
		createEntity(&(*iter));
	}
}

void GMBSPGameWorld::createEntity(BSPEntity* entity)
{
	D(d);
	D_BASE(GMGameWorld, db);

	BSPKeyValuePair* p = entity->epairs;
	const char* classname = nullptr;
	while (p)
	{
		if (strEqual("classname", p->key))
			classname = p->value;
		p = p->next;
	}

	Model* m = d->modelLoader.find(classname);
	if (!m)
	{
		gm_warning("model '%s' is not defined in model list, skipped.", classname);
		return;
	}

	// 不创建这个实体
	if (!m->create)
		return;

	GMBSPRenderData& rd = d->render.renderData();

	ASSERT(rd.entitiyObjects.find(entity) == rd.entitiyObjects.end());
	Object* coreObj = nullptr;

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
		d->render.createBox(m->extents, entity->origin, shader, &coreObj);
	}
	else
	{
		GamePackageBuffer buf;
		std::string fn(m->model);
		fn.append("/");
		fn.append(m->model);
		fn.append(".obj");

		std::string path = db->gamePackage->pathOf(PI_MODELS, fn.c_str());
		GMModelLoadSettings settings = {
			*db->gamePackage,
			m->extents,
			entity->origin,
			path.c_str(),
			m->model
		};
		
		if (!GMModelReader::load(settings, &coreObj))
		{
			gm_warning("parse model file failed.");
			return;
		}
	}

	GMEntityObject* obj = new GMEntityObject(coreObj);
	rd.entitiyObjects[entity] = obj;
	appendObjectAndInit(obj);
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
