#include "stdafx.h"
#include "gmbspphysicsstructs.h"

BEGIN_NS

GM_PRIVATE_OBJECT_ALIGNED(GMBSPPhysicsObject)
{
	GMBSPShapeProperties shapeProps;
};

GMBSPPhysicsObject::GMBSPPhysicsObject()
{
	GM_CREATE_DATA();
}

GMBSPPhysicsObject::~GMBSPPhysicsObject()
{

}

GMBSPShapeProperties& GMBSPPhysicsObject::shapeProperties()
{
	D(d); return d->shapeProps;
}

END_NS