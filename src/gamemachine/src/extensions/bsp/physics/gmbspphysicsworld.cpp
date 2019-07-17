#include "stdafx.h"
#include "gmbspphysicsworld.h"
#include "gmbspmove.h"
#include <gmphysicsworld.h>
#include "extensions/bsp/render/gmbspgameworld.h"

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

	//TODO TEST
	d->gravity = -600.f;
}

GMBSPPhysicsWorld::~GMBSPPhysicsWorld()
{
	D(d);
	for (auto& kv : d->objectMoves)
	{
		GM_delete(kv.second);
	}

	for (auto& kv : d->collisionObjects)
	{
		delete kv.second;
	}

	d->objectMoves.clear();
}

GMBSPPhysicsWorld::Data& GMBSPPhysicsWorld::physicsData()
{
	D(d);
	return *d;
}

void GMBSPPhysicsWorld::update(GMDuration dt, GMGameObject* obj)
{
	D(d);
	if (getEnabled())
	{
		BSPData& bsp = d->world->bspData();
		GMPhysicsObject* phy = obj->getPhysicsObject();
		if (phy)
		{
			GMBSPMove* move = getMove(phy);
			move->move(dt);
		}
	}
}

void GMBSPPhysicsWorld::applyMove(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args)
{
	GMBSPMove* move = getMove(phy);
	move->applyMove(args);
}

void GMBSPPhysicsWorld::applyJump(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args)
{
	GMBSPMove* move = getMove(phy);
	move->applyJump(args.speed);
}

void GMBSPPhysicsWorld::initBSPPhysicsWorld()
{
	generatePhysicsPlaneData();
	generatePhysicsBrushSideData();
	generatePhysicsBrushData();
	generatePhysicsPatches();
}

GMBSPMove* GMBSPPhysicsWorld::getMove(GMPhysicsObject* o)
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

	GM_ASSERT(m);
	return m;
}

void GMBSPPhysicsWorld::generatePhysicsPlaneData()
{
	D(d);
	GMFloat4 f4_normal;
	BSPData& bsp = d->world->bspData();
	d->planes.resize(bsp.numplanes);
	for (GMint32 i = 0; i < bsp.numplanes; i++)
	{
		bsp.planes[i].getNormal().loadFloat4(f4_normal);
		d->planes[i] = bsp.planes[i];
		d->planes[i].planeType = PlaneTypeForNormal(f4_normal);
		d->planes[i].signbits = signbitsForNormal(GMVec4(d->planes[i].getNormal(), 0));
	}
}

void GMBSPPhysicsWorld::generatePhysicsBrushSideData()
{
	D(d);
	BSPData& bsp = d->world->bspData();
	d->brushsides.resize(bsp.numbrushsides);
	for (GMint32 i = 0; i < bsp.numbrushsides; i++)
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
	for (GMint32 i = 0; i < bsp.numbrushes; i++)
	{
		GMBSP_Physics_Brush* b = &d->brushes[i];
		b->checkcount = 0;
		b->brush = &bsp.brushes[i];
		b->sides = &d->brushsides[b->brush->firstSide];
		b->contents = bsp.shaders[b->brush->shaderNum].contentFlags;
		b->bounds[0] = GMVec3(-b->sides[0].plane->getIntercept(), -b->sides[2].plane->getIntercept(), -b->sides[4].plane->getIntercept());
		b->bounds[1] = GMVec3(b->sides[1].plane->getIntercept(), b->sides[3].plane->getIntercept(), b->sides[5].plane->getIntercept());
	}
}

void GMBSPPhysicsWorld::generatePhysicsPatches()
{
	D(d);
	BSPData& bsp = d->world->bspData();
	// scan through all the surfaces, but only load patches,
	// not planar faces

	d->patch.alloc(bsp.numDrawSurfaces);
	for (GMint32 i = 0; i < bsp.numDrawSurfaces; i++)
	{
		if (bsp.drawSurfaces[i].surfaceType != MST_PATCH)
			continue;

		GMint32 width = bsp.drawSurfaces[i].patchWidth, height = bsp.drawSurfaces[i].patchHeight;
		GMint32 c = width * height;
		AlignedVector<GMVec3> points;
		points.resize(c);
		GMBSPDrawVertices* v = &bsp.vertices[bsp.drawSurfaces[i].firstVert];
		for (GMint32 j = 0; j < c; j++, v++)
		{
			points[j] = v->xyz;
		}

		GMBSP_Physics_Patch* patch = new GMBSP_Physics_Patch();
		patch->surface = &bsp.drawSurfaces[i];
		patch->shader = &bsp.shaders[patch->surface->shaderNum];
		d->patch.generatePatchCollide(i, width, height, points.data(), patch);
	}
}