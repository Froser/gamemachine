#include "stdafx.h"
#include "bspphysicsworld.h"
#include "gmengine/elements/bspgameworld.h"
#include "gmengine/controllers/gameloop.h"
#include "bspmove.h"
#include "gmphysics/collisionobjectfactory.h"

//class
BSPPhysicsWorld::BSPPhysicsWorld(GameWorld* world)
	: PhysicsWorld(world)
{
	D(d);
	d.world = static_cast<BSPGameWorld*>(world);
	d.trace.initTrace(&d.world->bspData(), this);

	d.camera = CollisionObjectFactory::defaultCamera();

	//TODO TEST
	d.gravity = -600.f;
}

BSPPhysicsWorld::~BSPPhysicsWorld()
{
	D(d);
	for (auto iter = d.objectMoves.begin(); iter != d.objectMoves.end(); iter++)
	{
		delete iter->second;
	}
	d.objectMoves.clear();
}

BSPPhysicsWorldData& BSPPhysicsWorld::physicsData()
{
	D(d);
	return d;
}

void BSPPhysicsWorld::simulate()
{
	D(d);
	BSPData& bsp = d.world->bspData();

	BSPMove* move = getMove(&d.camera);
	move->move();
}

CollisionObject* BSPPhysicsWorld::find(GameObject* obj)
{
	D(d);
	// 优先查找视角位置
	if (d.camera.object == obj)
		return &d.camera;

	return nullptr;
}

void BSPPhysicsWorld::initBSPPhysicsWorld()
{
	generatePhysicsPlaneData();
	generatePhysicsBrushSideData();
	generatePhysicsBrushData();
	generatePhysicsPatches();
}

void BSPPhysicsWorld::setCamera(GameObject* obj)
{
	D(d);
	d.camera.object = obj;
}

BSPMove* BSPPhysicsWorld::getMove(CollisionObject* o)
{
	D(d);
	BSPMove* m = nullptr;
	if (d.objectMoves.find(o) == d.objectMoves.end())
	{
		m = new BSPMove(this, o);
		d.objectMoves[o] = m;
	}
	else
	{
		m = d.objectMoves[o];
	}

	ASSERT(m);
	return m;
}

void BSPPhysicsWorld::generatePhysicsPlaneData()
{
	D(d);
	BSPData& bsp = d.world->bspData();
	d.planes.resize(bsp.numplanes);
	for (GMint i = 0; i < bsp.numplanes; i++)
	{
		d.planes[i] = bsp.planes[i];
		d.planes[i].planeType = PlaneTypeForNormal(bsp.planes[i].normal);
		d.planes[i].signbits = 0;
		for (GMint j = 0; j < 3; j++)
		{
			if (bsp.planes[i].normal[j] < 0)
				d.planes[i].signbits |= 1 << j;
		}
	}
}

void BSPPhysicsWorld::generatePhysicsBrushSideData()
{
	D(d);
	BSPData& bsp = d.world->bspData();
	d.brushsides.resize(bsp.numbrushsides);
	for (GMint i = 0; i < bsp.numbrushsides; i++)
	{
		BSP_Physics_BrushSide* bs = &d.brushsides[i];
		bs->side = &bsp.brushsides[i];
		bs->plane = &d.planes[bs->side->planeNum];
		bs->surfaceFlags = bsp.shaders[bs->side->shaderNum].surfaceFlags;
	}
}

void BSPPhysicsWorld::generatePhysicsBrushData()
{
	D(d);
	BSPData& bsp = d.world->bspData();
	d.brushes.resize(bsp.numbrushes);
	for (GMint i = 0; i < bsp.numbrushes; i++)
	{
		BSP_Physics_Brush* b = &d.brushes[i];
		b->checkcount = 0;
		b->brush = &bsp.brushes[i];
		b->sides = &d.brushsides[b->brush->firstSide];
		b->contents = bsp.shaders[b->brush->shaderNum].contentFlags;
		b->bounds[0][0] = -b->sides[0].plane->intercept;
		b->bounds[1][0] = b->sides[1].plane->intercept;
		b->bounds[0][1] = -b->sides[2].plane->intercept;
		b->bounds[1][1] = b->sides[3].plane->intercept;
		b->bounds[0][2] = -b->sides[4].plane->intercept;
		b->bounds[1][2] = b->sides[5].plane->intercept;
	}
}

void BSPPhysicsWorld::generatePhysicsPatches()
{
	D(d);
	BSPData& bsp = d.world->bspData();
	// scan through all the surfaces, but only load patches,
	// not planar faces

	d.patch.alloc(bsp.numDrawSurfaces);
	for (GMint i = 0; i < bsp.numDrawSurfaces; i++)
	{
		if (bsp.drawSurfaces[i].surfaceType != MST_PATCH)
			continue;

		GMint width = bsp.drawSurfaces[i].patchWidth, height = bsp.drawSurfaces[i].patchHeight;
		GMint c = width * height;
		std::vector<vmath::vec3> points;
		points.resize(c);
		BSPDrawVertices* v = &bsp.vertices[bsp.drawSurfaces[i].firstVert];
		for (GMint j = 0; j < c; j++, v++)
		{
			points[j] = v->xyz;
		}

		BSP_Physics_Patch* patch = GM_new<BSP_Physics_Patch>();
		patch->surface = &bsp.drawSurfaces[i];
		patch->shader = &bsp.shaders[patch->surface->shaderNum];
		d.patch.generatePatchCollide(i, width, height, points.data(), patch);
	}
}