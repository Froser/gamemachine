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
#include "utilities\plane.h"

#define M(x, y, z) makeVector(x, y, z, getUpAxis())

BSPGameWorld::BSPGameWorld(GMfloat scaling)
{
	D(d);
	D_BASE(GameWorld, dbase);
	d.scaling = scaling;
	dbase.upAxis = Z_AXIS;
}

void BSPGameWorld::loadBSP(const char* bspPath)
{
	D(d);
	d.bsp.loadBsp(bspPath);
	importBSP();
}

void BSPGameWorld::renderGameWorld()
{
	D(d);
	calculateVisibleFaces();
	drawFaces();
}

void BSPGameWorld::calculateVisibleFaces()
{
	D(d);
	D_BASE(GameWorld, dbase);
	Character* character = getMajorCharacter();
	PositionState pos = character->getPositionState();
	BSPData& bsp = d.bsp.bspData();

	GMint cameraLeaf = calculateCameraLeaf(M(pos.positionX, pos.positionY, pos.positionZ));
	GMint cameraCluster = bsp.leafs[cameraLeaf].cluster;

	for (int i = 0; i < bsp.numleafs; ++i)
	{
		//if the leaf is not in the PVS, continue
		if (!isClusterVisible(cameraCluster, bsp.leafs[i].cluster))
			continue;

		//if this leaf does not lie in the frustum, continue
		//if (!frustum.isBoundingBoxInside(bsp.leafs[i]))
		//	continue;

		//loop through faces in this leaf and mark them to be drawn
		for (int j = 0; j < bsp.leafs[i].numLeafSurfaces; ++j)
		{
			bsp.facesToDraw.set(bsp.leafsurfaces[bsp.leafs[i].firstLeafSurface + j]);
		}
	}
}

int BSPGameWorld::calculateCameraLeaf(const btVector3& cameraPosition)
{
	D(d);
	BSPData& bsp = d.bsp.bspData();

	int currentNode = 0;

	//loop until we find a negative index
	while (currentNode >= 0)
	{
		//if the camera is in front of the plane for this node, assign i to be the front node
		BSPPlane& bspplane = bsp.planes[bsp.nodes[currentNode].planeNum];
		Plane plane(btVector3(bspplane.normal[0], bspplane.normal[1], bspplane.normal[2]), bspplane.dist);

		if (plane.classifyPoint(cameraPosition) == POINT_IN_FRONT_OF_PLANE)
			currentNode = bsp.nodes[currentNode].children[0]; //front
		else
			currentNode = bsp.nodes[currentNode].children[1]; //back
	}

	//return leaf index
	return ~currentNode;
}

int BSPGameWorld::isClusterVisible(int cameraCluster, int testCluster)
{
	/*
	int index = cameraCluster * visibilityData.bytesPerCluster + testCluster / 8;

	int returnValue = visibilityData.bitset[index] & (1 << (testCluster & 7));

	return returnValue;
	*/
	return true;
}

void BSPGameWorld::drawFaces()
{

}

void BSPGameWorld::importBSP()
{
	D(d);
	const BSPData& bsp = d.bsp.bspData();

	importWorldSpawn();
	importPlayer();
	//importLeafs();
	initialize();
}

void BSPGameWorld::importWorldSpawn()
{
	D(d);
	BSPEntity* entity;
	// ASSERT: 第一个entity一定是worldspawn
	if (d.bsp.findEntityByClassName("worldspawn", entity))
	{
		{
			GMfloat ambient;
			bool b = d.bsp.floatForKey(entity, "ambient", &ambient);
			if (b)
			{
				GameLight* ambientLight;
				IFactory* factory = getGameMachine()->getFactory();
				factory->createLight(Ambient, &ambientLight);
				if (ambientLight)
				{
					ambientLight->setId(0);
					//ambientLight->setColor(btVector3(ambient * .01f, ambient * .01f, ambient * .01f));
					ambientLight->setColor(btVector3(.5, .5, .5));
					ambientLight->setPosition(btVector3(0, 0, 0));
					ambientLight->setRange(0);
					ambientLight->setWorld(this);
					ambientLight->setShadowSource(false);
					appendLight(ambientLight);
				}
			}
		}

		{
			GMfloat gravity;
			bool b = d.bsp.floatForKey(entity, "gravity", &gravity);
			setGravity(0, 0, 0);
		}
	}
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
		playerStart.setOrigin(M(origin[0], origin[1], origin[2]));
		Character* character = new Character(playerStart, d.scaling, 10, 10);

		character->setMoveSpeed(500);
		character->setCanFreeMove(true);

		appendObject(character);
		setMajorCharacter(character);

		//TODO
		/*
		character->setJumpSpeed(btVector3(character.jumpSpeed[0], character.jumpSpeed[1], character.jumpSpeed[2]));
		character->setFallSpeed(character.fallSpeed);
		character->setEyeOffset(character.eyeOffset);
		*/
	}
}

void BSPGameWorld::importLeafs()
{
	D(d);
	BSPData& bsp = d.bsp.bspData();
	for (int i = 0; i < bsp.numleafs; i++)
	{
		bool isValidBrush = false;

		BSPLeaf& leaf = bsp.leafs[i];

		for (int b = 0; b < leaf.numLeafBrushes; b++)
		{
			btAlignedObjectArray<btVector3> planeEquations;

			int brushid = bsp.leafbrushes[leaf.firstLeafBrush + b];

			BSPBrush& brush = bsp.brushes[brushid];
			if (brush.shaderNum != -1)
			{
				if (bsp.shaders[brush.shaderNum].contentFlags & CONTENTS_SOLID)
				{
					brush.shaderNum = -1;

					for (int p = 0; p < brush.numSides; p++)
					{
						int sideid = brush.firstSide + p;
						BSPBrushSide& brushside = bsp.brushsides[sideid];
						int planeid = brushside.planeNum;
						BSPPlane& plane = bsp.planes[planeid];
						btVector3 planeEq;
						planeEq.setValue(
							plane.normal[0],
							plane.normal[1],
							plane.normal[2]);
						planeEq[3] = d.scaling* - plane.dist;

						planeEquations.push_back(planeEq);
						isValidBrush = true;
					}

					if (isValidBrush)
					{
						btAlignedObjectArray<btVector3> vertices;
						Geometry::getVerticesFromPlaneEquations(planeEquations, vertices, getUpAxis());

						bool isEntity = false;
						btVector3 entityTarget(0.f, 0.f, 0.f);
						addConvexVerticesCollider(vertices, isEntity, entityTarget);
					}
				}
			}
		}
	}
}

void BSPGameWorld::addConvexVerticesCollider(btAlignedObjectArray<btVector3>& vertices, bool isEntity, const btVector3& entityTargetLocation)
{
	static int i = 0;
	static btVector4 sColors[4] =
	{
		btVector4(1,0.7,0.7,1),
		btVector4(1,1,0.7,1),
		btVector4(0.7,1,0.7,1),
		btVector4(0.7,1,1,1),
	};

	if (vertices.size() > 0)
	{
		btVector4& vec = sColors[i % 4];

		Material m = { 0 };
		m.Ka[0] = vec[0];
		m.Ka[1] = vec[1];
		m.Ka[2] = vec[2];
		i++;

		ConvexHullGameObject* sp = new ConvexHullGameObject(vertices[0], vertices.size(), m);
		appendObject(sp);
	}
}