#include "stdafx.h"
#include "bspgameworld.h"
#include "utilities\log.h"
#include "character.h"
#include "LinearMath\btGeometryUtil.h"
#include "convexhullgameobject.h"
#include "gamelight.h"
#include "gmengine/controller/factory.h"
#include "gmengine/controller/gamemachine.h"
#include "utilities/algorithm.h"
#include "utilities/plane.h"
#include "gmengine/controller/resource_container.h"
#include "gmdatacore/imagereader/imagereader.h"
#include "utilities/path.h"
#include "gmdatacore/imagebuffer.h"

BSPGameWorld::BSPGameWorld()
{
	D(d);
	D_BASE(GameWorld, dbase);
}

void BSPGameWorld::loadBSP(const char* bspPath)
{
	D(d);
	d.bspWorkingDirectory = Path::directoryName(bspPath);
	d.bsp.loadBsp(bspPath);
	importBSP();
}

void BSPGameWorld::updateCamera()
{
	D(d);

	IGraphicEngine* engine = getGraphicEngine();
	Character* character = getMajorCharacter();
	character->updateCamera();
	CameraLookAt& lookAt = character->getLookAt();
	engine->updateCameraView(lookAt);
}

void BSPGameWorld::renderGameWorld()
{
	D(d);
	IGraphicEngine* engine = getGraphicEngine();
	engine->newFrame();

	updateCamera();
	calculateVisibleFaces();
	drawFaces();

	engine->drawObjects(d.drawingList);
}

void BSPGameWorld::calculateVisibleFaces()
{
	D(d);
	D_BASE(GameWorld, dbase);
	Character* character = getMajorCharacter();
	PositionState pos = character->getPositionState();
	BSPData& bsp = d.bsp.bspData();

	bsp.facesToDraw.clearAll();
	GMint cameraLeaf = calculateCameraLeaf(vmath::vec3(pos.positionX, pos.positionY, pos.positionZ));
	GMint cameraCluster = bsp.leafs[cameraLeaf].cluster;

	for (int i = 0; i < bsp.numleafs; ++i)
	{
		//if the leaf is not in the PVS, continue
		if (!isClusterVisible(cameraCluster, bsp.leafs[i].cluster))
			continue;

		//if this leaf does not lie in the frustum, continue
		Character* character = getMajorCharacter();
		if (!character->getFrustum().isBoundingBoxInside(bsp.drawingLeafs[i].boundingBoxVertices))
			continue;

		//loop through faces in this leaf and mark them to be drawn
		for (int j = 0; j < bsp.leafs[i].numLeafSurfaces; ++j)
		{
			bsp.facesToDraw.set(bsp.leafsurfaces[bsp.leafs[i].firstLeafSurface + j]);
		}
	}
}

int BSPGameWorld::calculateCameraLeaf(const vmath::vec3& cameraPosition)
{
	D(d);
	BSPData& bsp = d.bsp.bspData();

	int currentNode = 0;

	//loop until we find a negative index
	while (currentNode >= 0)
	{
		//if the camera is in front of the plane for this node, assign i to be the front node
		if (bsp.drawingPlanes[bsp.nodes[currentNode].planeNum].classifyPoint(cameraPosition) == POINT_IN_FRONT_OF_PLANE)
			currentNode = bsp.nodes[currentNode].children[0]; //front
		else
			currentNode = bsp.nodes[currentNode].children[1]; //back
	}

	//return leaf index
	return ~currentNode;
}

int BSPGameWorld::isClusterVisible(int cameraCluster, int testCluster)
{
	D(d);
	BSPData& bsp = d.bsp.bspData();

	int index = cameraCluster * bsp.visibilityData.bytesPerCluster + testCluster / 8;

	if (index < 0)
		return 1;

	int returnValue = bsp.visibilityData.bitset[index] & (1 << (testCluster & 7));
	return returnValue;
}

void BSPGameWorld::drawFaces()
{
	D(d);
	BSPData& bsp = d.bsp.bspData();
	d.drawingList.clear();

	//loop through faces
	for (GMint i = 0; i < bsp.numDrawSurfaces; ++i)
	{
		//if this face is to be drawn, draw it
		if (bsp.facesToDraw.isSet(i))
			drawFace(i);
	}
}

void BSPGameWorld::drawFace(GMint idx)
{
	D(d);
	BSPData& bsp = d.bsp.bspData();
	//look this face up in the face directory
	if (bsp.drawingFaceDirectory[idx].faceType == 0)
		return;

	if (bsp.drawingFaceDirectory[idx].faceType == MST_PLANAR)
		drawPolygonFace(bsp.drawingFaceDirectory[idx].typeFaceNumber);

	if (bsp.drawingFaceDirectory[idx].faceType == MST_TRIANGLE_SOUP)
		drawMeshFace(bsp.drawingFaceDirectory[idx].typeFaceNumber);

	if (bsp.drawingFaceDirectory[idx].faceType == MST_PATCH)
		drawPatch(bsp.drawingFaceDirectory[idx].typeFaceNumber);
}

void BSPGameWorld::drawPolygonFace(int polygonFaceNumber)
{
	D(d);
	BSPData& bsp = d.bsp.bspData();
	auto& polygonFace = bsp.drawingPolygonFaces[polygonFaceNumber];

	GameObject* obj = nullptr;
	auto findResult = d.polygonFaceObjects.find(&polygonFace);
	if (findResult == d.polygonFaceObjects.end())
	{
		Material material = { 0 };
		material.Ka[0] = 1.0f; material.Ka[1] = 1.0f; material.Ka[2] = 1.0f;
		if (!setMaterialTexture(polygonFace.textureIndex, material))
			return;
		setMaterialLightmap(polygonFace.lightmapIndex, material);

		Object* coreObj = new Object();
		ChildObject* child = new ChildObject();
		Component* component = new Component(child);
		component->getMaterial() = material;
		component->beginFace();

		for (GMuint i = 0; i < polygonFace.numVertices; i++)
		{
			BSP_Drawing_Vertex& vertex = bsp.drawingVertices[i + polygonFace.firstVertexIndex];
			component->vertex(vertex.position[0], vertex.position[1], vertex.position[2]);
			component->uv(vertex.decalS, vertex.decalT);
			component->lightmap(vertex.lightmapS, vertex.lightmapT);
		}

		component->endFace();
		child->appendComponent(component);
		coreObj->append(child);
		obj = new ConvexHullGameObject(coreObj);

		d.polygonFaceObjects[&polygonFace] = obj;
		appendObjectAndInit(obj);
	}
	else
	{
		obj = (*findResult).second;
	}

	ASSERT(obj);
	obj->getReadyForRender(d.drawingList);
}

void BSPGameWorld::drawMeshFace(int meshFaceNumber)
{
	D(d);
	BSPData& bsp = d.bsp.bspData();
	auto& meshFace = bsp.drawingMeshFaces[meshFaceNumber];

	GameObject* obj = nullptr;
	auto findResult = d.meshFaceObjects.find(&meshFace);
	if (findResult == d.meshFaceObjects.end())
	{
		Material material = { 0 };
		material.Ka[0] = 1.0f; material.Ka[1] = 1.0f; material.Ka[2] = 1.0f;
		if (!setMaterialTexture(meshFace.textureIndex, material))
			return;
		setMaterialLightmap(meshFace.lightmapIndex, material);

		Object* coreObj = new Object();
		ChildObject* child = new ChildObject();
		child->setArrangementMode(ChildObject::Triangles);
		Component* component = new Component(child);
		component->getMaterial() = material;
		component->beginFace();

		GMuint* idxStart = (GMuint*)&bsp.drawIndexes[meshFace.firstMeshIndex];
		GMuint offset = meshFace.firstVertexIndex;
		for (int i = 0; i < meshFace.numMeshIndices; ++i)
		{
			GMuint idx = *(idxStart + i);
			BSP_Drawing_Vertex& vertex = bsp.drawingVertices[offset + idx];
			component->vertex(vertex.position[0], vertex.position[1], vertex.position[2]);
			component->uv(vertex.decalS, vertex.decalT);
			component->lightmap(vertex.lightmapS, vertex.lightmapT);
		}
		component->endFace();
		child->appendComponent(component);

		coreObj->append(child);
		obj = new ConvexHullGameObject(coreObj);
		d.meshFaceObjects[&meshFace] = obj;
		appendObjectAndInit(obj);
	}
	else
	{
		obj = (*findResult).second;
	}

	ASSERT(obj);
	obj->getReadyForRender(d.drawingList);
}

void BSPGameWorld::drawPatch(int patchNumber)
{
	D(d);
	BSPData& bsp = d.bsp.bspData();

	Material material = { 0 };
	material.Ka[0] = 1.0f; material.Ka[1] = 1.0f; material.Ka[2] = 1.0f;
	if (!setMaterialTexture(bsp.drawingPatches[patchNumber].textureIndex, material))
		return;
	setMaterialLightmap(bsp.drawingPatches[patchNumber].lightmapIndex, material);

	for (int i = 0; i < bsp.drawingPatches[patchNumber].numQuadraticPatches; ++i)
		draw(bsp.drawingPatches[patchNumber].quadraticPatches[i], material);
}

void BSPGameWorld::draw(BSP_Drawing_BiquadraticPatch& biqp, Material& material)
{
	D(d);
	GameObject* obj = nullptr;
	auto findResult = d.biquadraticPatchObjects.find(&biqp);
	if (findResult == d.biquadraticPatchObjects.end())
	{
		Object* coreObj = new Object();
		ChildObject* child = new ChildObject();
		child->setArrangementMode(ChildObject::Triangle_Strip);
		
		Component* component = new Component(child);
		component->getMaterial() = material;

		int numVertices = 2 * (biqp.tesselation + 1);
		for (int row = 0; row < biqp.tesselation; ++row)
		{
			component->beginFace();
			GLuint* idxStart = &biqp.indices[row * 2 * (biqp.tesselation + 1)];
			for (int i = 0; i < numVertices; i++)
			{
				GMuint idx = *(idxStart + i);
				BSP_Drawing_Vertex& vertex = biqp.vertices[idx];

				component->vertex(vertex.position[0], vertex.position[1], vertex.position[2]);
				component->uv(vertex.decalS, vertex.decalT);
				component->lightmap(vertex.lightmapS, vertex.lightmapT);
			}
			component->endFace();

		}
		child->appendComponent(component);

		coreObj->append(child);
		obj = new ConvexHullGameObject(coreObj);
		d.biquadraticPatchObjects[&biqp] = obj;
		appendObjectAndInit(obj);
	}
	else
	{
		obj = (*findResult).second;
	}

	ASSERT(obj);
	obj->getReadyForRender(d.drawingList);
}

bool BSPGameWorld::setMaterialTexture(ID textureid, REF Material& m)
{
	D(d);
	ResourceContainer* rc = getGameMachine()->getGraphicEngine()->getResourceContainer();
	TextureContainer& tc = rc->getTextureContainer();
	const TextureContainer::TextureItem* item = tc.find(textureid);
	if (!item)
		return false;

	m.textures.autorelease = false;
	m.textures.texture[TEXTURE_INDEX_AMBIENT] = item->texture;
	return true;
}

void BSPGameWorld::setMaterialLightmap(ID lightmapid, REF Material& m)
{
	D(d);
	const GMint WHITE_LIGHTMAP = -1;
	ResourceContainer* rc = getGameMachine()->getGraphicEngine()->getResourceContainer();
	TextureContainer& tc = rc->getLightmapContainer();
	const TextureContainer::TextureItem* item = lightmapid >= 0 ? tc.find(lightmapid) : tc.find(WHITE_LIGHTMAP);

	m.textures.autorelease = false;
	m.textures.texture[TEXTURE_INDEX_LIGHTMAP] = item->texture;
}

void BSPGameWorld::importBSP()
{
	initTextures();
	initLightmaps();
	importWorldSpawn();
	importPlayer();
	initialize();
}

void BSPGameWorld::initTextures()
{
	D(d);
	BSPData& bsp = d.bsp.bspData();

	IFactory* factory = getGameMachine()->getFactory();
	ResourceContainer* rc = getGraphicEngine()->getResourceContainer();

	for (GMuint i = 0; i < bsp.numShaders; i++)
	{
		BSPShader& shader = bsp.shaders[i];
		ImageReader imgReader;
		Image* tex = nullptr;

		std::string fn = d.bspWorkingDirectory;
		fn.append(shader.shader);
		fn.append(".png");
		if (imgReader.load(fn.c_str(), &tex))
		{
			ITexture* texture;
			factory->createTexture(tex, &texture);

			TextureContainer::TextureItem item;
			item.id = i;
			item.texture = texture;
			rc->getTextureContainer().insert(item);
		}
	}

}

void BSPGameWorld::initLightmaps()
{
	D(d);
	BSPData& bsp = d.bsp.bspData();
	adjustLightmapGamma();

	IFactory* factory = getGameMachine()->getFactory();
	ResourceContainer* rc = getGraphicEngine()->getResourceContainer();

	const int BSP_LIGHTMAP_EXT = 128;
	const int BSP_LIGHTMAP_SIZE = BSP_LIGHTMAP_EXT * BSP_LIGHTMAP_EXT * 3 * sizeof(GMbyte);
	int numLightmaps = bsp.numLightBytes / (BSP_LIGHTMAP_SIZE * sizeof(GMbyte));

	for (GMuint i = 0; i < numLightmaps; i++)
	{
		GMbyte* lightmapBytes = bsp.lightBytes.data() + i * BSP_LIGHTMAP_SIZE;
		ImageBuffer* imgBuf = new ImageBuffer(BSP_LIGHTMAP_EXT, BSP_LIGHTMAP_EXT, BSP_LIGHTMAP_SIZE, lightmapBytes);
		ITexture* texture = nullptr;
		factory->createTexture(imgBuf, &texture);

		TextureContainer::TextureItem item;
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

		TextureContainer::TextureItem item;
		item.id = -1;
		item.texture = texture;
		rc->getLightmapContainer().insert(item);
	}
}

void BSPGameWorld::adjustLightmapGamma()
{
	D(d);
	BSPData& bsp = d.bsp.bspData();
	/*
	float gamma = 2.5f;
	for (int i = 0; i < bsp.numLightBytes; ++i)
	{
		for (int j = 0; j < 128 * 128; ++j)
		{
			float r, g, b;
			r = bsp.lightBytes[i].lightmapData[j * 3 + 0];
			g = bsp.lightBytes[i].lightmapData[j * 3 + 1];
			b = bsp.lightBytes[i].lightmapData[j * 3 + 2];

			r *= gamma / 255.0f;
			g *= gamma / 255.0f;
			b *= gamma / 255.0f;

			//find the value to scale back up
			float scale = 1.0f;
			float temp;
			if (r > 1.0f && (temp = (1.0f / r)) < scale) scale = temp;
			if (g > 1.0f && (temp = (1.0f / g)) < scale) scale = temp;
			if (b > 1.0f && (temp = (1.0f / b)) < scale) scale = temp;

			// scale up color values
			scale *= 255.0f;
			r *= scale;
			g *= scale;
			b *= scale;

			//fill data back in
			bsp.lightBytes[i].lightmapData[j * 3 + 0] = (GLubyte)r;
			bsp.lightBytes[i].lightmapData[j * 3 + 1] = (GLubyte)g;
			bsp.lightBytes[i].lightmapData[j * 3 + 2] = (GLubyte)b;
		}
	}
	*/
}

void BSPGameWorld::importWorldSpawn()
{
	D(d);
	BSPEntity* entity;
	// ASSERT: 第一个entity一定是worldspawn
#if 0
	if (d.bsp.findEntityByClassName("worldspawn", entity))
	{
		{
			GMfloat ambient;
			bool b = d.bsp.floatForKey(entity, "ambient", &ambient);
			if (b)
			{
#endif
				GameLight* ambientLight;
				IFactory* factory = getGameMachine()->getFactory();
				factory->createLight(Ambient, &ambientLight);
				if (ambientLight)
				{
					ambientLight->setId(0);
					//ambientLight->setColor(btVector3(ambient * .01f, ambient * .01f, ambient * .01f));
					ambientLight->setColor(btVector3(1, 1, 1));
					ambientLight->setPosition(btVector3(0, 0, 0));
					ambientLight->setRange(0);
					ambientLight->setWorld(this);
					ambientLight->setShadowSource(false);
					appendLight(ambientLight);
				}
#if 0
			}
		}

		{
			GMfloat gravity;
			bool b = d.bsp.floatForKey(entity, "gravity", &gravity);
			setGravity(0, 0, 0);
		}
	}
#endif

	setGravity(0, -800, 0);
}

void BSPGameWorld::importPlayer()
{
	D(d);
	BSPEntity* entity;
	if (d.bsp.findEntityByClassName("info_player_deathmatch", entity))
	{
		BSPVector3 origin;
		d.bsp.vectorForKey(entity, "origin", origin);
		LOG_INFO("found playerstart\n");
		btTransform playerStart;
		playerStart.setIdentity();
		//playerStart.setOrigin(M(origin[0], origin[1], origin[2]));
		playerStart.setOrigin(btVector3(0, 0.875, 0));
		Character* character = new Character(playerStart, .6, .1, .1);

		character->setMoveSpeed(3);
		character->setFallSpeed(400);
		character->setJumpSpeed(btVector3(0, 50, 0));
		character->setCanFreeMove(false);

		appendObjectAndInit(character);
		setMajorCharacter(character);

		//TODO
		/*
		character->setJumpSpeed(btVector3(character.jumpSpeed[0], character.jumpSpeed[1], character.jumpSpeed[2]));
		character->setFallSpeed(character.fallSpeed);
		character->setEyeOffset(character.eyeOffset);
		*/
	}
}