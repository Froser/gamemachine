#ifndef __CONVEXHULLGAMEOBJECT_H__
#define __CONVEXHULLGAMEOBJECT_H__
#include "common.h"
#include "gameobject.h"
#include "utilities/autoptr.h"
#include "rigidgameobject.h"
BEGIN_NS

class ChildObject;
class ConvexHullGameObject : public RigidGameObject
{
public:
	ConvexHullGameObject(AUTORELEASE Object* obj);
	ConvexHullGameObject(GMfloat* vertices, GMuint count, Material& material);

public:
	virtual void getReadyForRender(DrawingList& list);
	virtual btTransform getRuntimeTransform();
	void initPhysicsAfterCollisionObjectCreated() override;

private:
	virtual btCollisionShape* createCollisionShape() override;
	void createTriangleMesh();

private:
	bool m_fromVertices;
	Material m_material;
	GMfloat* m_vertices;
	GMuint m_verticesCount;
};

END_NS
#endif