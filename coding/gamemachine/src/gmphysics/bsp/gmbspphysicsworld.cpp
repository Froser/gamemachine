#include "stdafx.h"
#include "gmbspphysicsworld.h"
#include "gmbspmove.h"
#include "gmengine/gmbspgameworld.h"
#include "gmphysics/collisionobjectfactory.h"

//class
GMBSPPhysicsWorld::GMBSPPhysicsWorld(GMGameWorld* world)
	: GMPhysicsWorld(world)
{
	D(d);
	d->world = static_cast<GMBSPGameWorld*>(world);
	d->trace.initTrace(
		&d->world->bspData(), 
		&d->world->getEntities(),
		&d->world->renderData().entitiyObjects, 
		this
	);

	d->camera = CollisionObjectFactory::defaultCamera();

	//TODO TEST
	d->gravity = -600.f;
}

GMBSPPhysicsWorld::~GMBSPPhysicsWorld()
{
	D(d);
	for (auto iter = d->objectMoves.begin(); iter != d->objectMoves.end(); iter++)
	{
		delete iter->second;
	}
	d->objectMoves.clear();
}

GMBSPPhysicsWorld::Data& GMBSPPhysicsWorld::physicsData()
{
	D(d);
	return *d;
}

void GMBSPPhysicsWorld::simulate()
{
	D(d);
	BSPData& bsp = d->world->bspData();

	GMBSPMove* move = getMove(&d->camera);
	move->move();
}

GMCollisionObject* GMBSPPhysicsWorld::find(GMGameObject* obj)
{
	D(d);
	// 优先查找视角位置
	if (d->camera.object == obj)
		return &d->camera;

	return nullptr;
}

void GMBSPPhysicsWorld::sendCommand(GMCollisionObject* obj, const CommandParams& dataParam)
{
	D(d);
	ASSERT(dataParam.size() == 1);
	GMCommand cmd = (*dataParam.begin()).first;
	GMBSPMove* move = getMove(&d->camera);
	move->sendCommand(cmd, dataParam);
}

void GMBSPPhysicsWorld::initBSPPhysicsWorld()
{
	generatePhysicsPlaneData();
	generatePhysicsBrushSideData();
	generatePhysicsBrushData();
	generatePhysicsPatches();
}

void GMBSPPhysicsWorld::setCamera(GMGameObject* obj)
{
	D(d);
	d->camera.object = obj;
}

GMBSPMove* GMBSPPhysicsWorld::getMove(GMCollisionObject* o)
{
	D(d);
	GMBSPMove* m = nullptr;
	if (d->objectMoves.find(o) == d->objectMoves.end())
	{
		m = new GMBSPMove(this, o);
		d->objectMoves[o] = m;
	}
	else
	{
		m = d->objectMoves[o];
	}

	ASSERT(m);
	return m;
}

void GMBSPPhysicsWorld::generatePhysicsPlaneData()
{
	D(d);
	BSPData& bsp = d->world->bspData();
	d->planes.resize(bsp.numplanes);
	for (GMint i = 0; i < bsp.numplanes; i++)
	{
		d->planes[i] = bsp.planes[i];
		d->planes[i].planeType = PlaneTypeForNormal(bsp.planes[i].normal);
		d->planes[i].signbits = 0;
		for (GMint j = 0; j < 3; j++)
		{
			if (bsp.planes[i].normal[j] < 0)
				d->planes[i].signbits |= 1 << j;
		}
	}
}

void GMBSPPhysicsWorld::generatePhysicsBrushSideData()
{
	D(d);
	BSPData& bsp = d->world->bspData();
	d->brushsides.resize(bsp.numbrushsides);
	for (GMint i = 0; i < bsp.numbrushsides; i++)
	{
		GMBSP_Physics_BrushSide* bs = &d->brushsides[i];
		bs->side = &bsp.brushsides[i];
		bs->plane = &d->planes[bs->side->planeNum];
		bs->surfaceFlags = bsp.shaders[bs->side->shaderNum].surfaceFlags;
	}
}

void GMBSPPhysicsWorld::generatePhysicsBrushData()
{
	D(d);
	BSPData& bsp = d->world->bspData();
	d->brushes.resize(bsp.numbrushes);
	for (GMint i = 0; i < bsp.numbrushes; i++)
	{
		GMBSP_Physics_Brush* b = &d->brushes[i];
		b->checkcount = 0;
		b->brush = &bsp.brushes[i];
		b->sides = &d->brushsides[b->brush->firstSide];
		b->contents = bsp.shaders[b->brush->shaderNum].contentFlags;
		b->bounds[0][0] = -b->sides[0].plane->intercept;
		b->bounds[1][0] = b->sides[1].plane->intercept;
		b->bounds[0][1] = -b->sides[2].plane->intercept;
		b->bounds[1][1] = b->sides[3].plane->intercept;
		b->bounds[0][2] = -b->sides[4].plane->intercept;
		b->bounds[1][2] = b->sides[5].plane->intercept;
	}
}

void GMBSPPhysicsWorld::generatePhysicsPatches()
{
	D(d);
	BSPData& bsp = d->world->bspData();
	// scan through all the surfaces, but only load patches,
	// not planar faces

	d->patch.alloc(bsp.numDrawSurfaces);
	for (GMint i = 0; i < bsp.numDrawSurfaces; i++)
	{
		if (bsp.drawSurfaces[i].surfaceType != MST_PATCH)
			continue;

		GMint width = bsp.drawSurfaces[i].patchWidth, height = bsp.drawSurfaces[i].patchHeight;
		GMint c = width * height;
		AlignedVector<linear_math::Vector3> points;
		points.resize(c);
		GMBSPDrawVertices* v = &bsp.vertices[bsp.drawSurfaces[i].firstVert];
		for (GMint j = 0; j < c; j++, v++)
		{
			points[j] = v->xyz;
		}

		GMBSP_Physics_Patch* patch;
		GM_new<GMBSP_Physics_Patch>(&patch);
		patch->surface = &bsp.drawSurfaces[i];
		patch->shader = &bsp.shaders[patch->surface->shaderNum];
		d->patch.generatePatchCollide(i, width, height, points.data(), patch);
	}
}