#include "stdafx.h"
#include "convexhullgameobject.h"
#include "gmdatacore/object.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"

static void push_back(std::vector<Object::DataType>& vector, const btVector3& pos)
{
	vector.push_back(pos[0]);
	vector.push_back(pos[1]);
	vector.push_back(pos[2]);
	vector.push_back(1);
}

static void collisionShape2TriangleMesh(btCollisionShape* collisionShape,
	const btTransform& parentTransform,
	std::vector<Object::DataType>& vertexPositions,
	std::vector<Object::DataType>& vertexNormals,
	std::vector<GMuint>& indicesOut
)
{
	//todo: support all collision shape types
	switch (collisionShape->getShapeType())
	{
	case SOFTBODY_SHAPE_PROXYTYPE:
	{
		//skip the soft body collision shape for now
		break;
	}
	case STATIC_PLANE_PROXYTYPE:
	{
		//draw a box, oriented along the plane normal
		const btStaticPlaneShape* staticPlaneShape = static_cast<const btStaticPlaneShape*>(collisionShape);
		btScalar planeConst = staticPlaneShape->getPlaneConstant();
		const btVector3& planeNormal = staticPlaneShape->getPlaneNormal();
		btVector3 planeOrigin = planeNormal * planeConst;
		btVector3 vec0, vec1;
		btPlaneSpace1(planeNormal, vec0, vec1);
		btScalar vecLen = 100.f;
		btVector3 verts[4];

		verts[0] = planeOrigin + vec0*vecLen + vec1*vecLen;
		verts[1] = planeOrigin - vec0*vecLen + vec1*vecLen;
		verts[2] = planeOrigin - vec0*vecLen - vec1*vecLen;
		verts[3] = planeOrigin + vec0*vecLen - vec1*vecLen;

		int startIndex = vertexPositions.size();
		indicesOut.push_back(startIndex + 0);
		indicesOut.push_back(startIndex + 1);
		indicesOut.push_back(startIndex + 2);
		indicesOut.push_back(startIndex + 0);
		indicesOut.push_back(startIndex + 2);
		indicesOut.push_back(startIndex + 3);

		btVector3 triNormal = parentTransform.getBasis()*planeNormal;

		for (int i = 0; i < 4; i++)
		{
			btVector3 vtxPos;
			btVector3 pos = parentTransform*verts[i];
			push_back(vertexPositions, pos);
			push_back(vertexNormals, triNormal);
		}
		break;
	}
	case TRIANGLE_MESH_SHAPE_PROXYTYPE:
	{
		btBvhTriangleMeshShape* trimesh = (btBvhTriangleMeshShape*)collisionShape;
		btVector3 trimeshScaling = trimesh->getLocalScaling();
		btStridingMeshInterface* meshInterface = trimesh->getMeshInterface();
		btAlignedObjectArray<btVector3> vertices;
		btAlignedObjectArray<int> indices;

		for (int partId = 0; partId < meshInterface->getNumSubParts(); partId++)
		{
			const unsigned char *vertexbase = 0;
			int numverts = 0;
			PHY_ScalarType type = PHY_INTEGER;
			int stride = 0;
			const unsigned char *indexbase = 0;
			int indexstride = 0;
			int numfaces = 0;
			PHY_ScalarType indicestype = PHY_INTEGER;
			//PHY_ScalarType indexType=0;

			btVector3 triangleVerts[3];
			meshInterface->getLockedReadOnlyVertexIndexBase(&vertexbase, numverts, type, stride, &indexbase, indexstride, numfaces, indicestype, partId);
			btVector3 aabbMin, aabbMax;

			for (int triangleIndex = 0; triangleIndex < numfaces; triangleIndex++)
			{
				unsigned int* gfxbase = (unsigned int*)(indexbase + triangleIndex*indexstride);

				for (int j = 2; j >= 0; j--)
				{

					int graphicsindex = indicestype == PHY_SHORT ? ((unsigned short*)gfxbase)[j] : gfxbase[j];
					if (type == PHY_FLOAT)
					{
						GMfloat* graphicsbase = (GMfloat*)(vertexbase + graphicsindex*stride);
						triangleVerts[j] = btVector3(
							graphicsbase[0] * trimeshScaling.getX(),
							graphicsbase[1] * trimeshScaling.getY(),
							graphicsbase[2] * trimeshScaling.getZ());
					}
					else
					{
						double* graphicsbase = (double*)(vertexbase + graphicsindex*stride);
						triangleVerts[j] = btVector3(btScalar(graphicsbase[0] * trimeshScaling.getX()),
							btScalar(graphicsbase[1] * trimeshScaling.getY()),
							btScalar(graphicsbase[2] * trimeshScaling.getZ()));
					}
				}
				indices.push_back(vertices.size());
				vertices.push_back(triangleVerts[0]);
				indices.push_back(vertices.size());
				vertices.push_back(triangleVerts[1]);
				indices.push_back(vertices.size());
				vertices.push_back(triangleVerts[2]);

				btVector3 triNormal = (triangleVerts[1] - triangleVerts[0]).cross(triangleVerts[2] - triangleVerts[0]);
				triNormal.normalize();

				for (int v = 0; v < 3; v++)
				{

					btVector3 pos = parentTransform*triangleVerts[v];
					indicesOut.push_back(vertexPositions.size());
					push_back(vertexPositions, pos);
					push_back(vertexNormals, triNormal);
				}
			}
		}

		break;
	}
	default:
	{
		if (collisionShape->isConvex())
		{
			btConvexShape* convex = (btConvexShape*)collisionShape;
			{
				btShapeHull* hull = new btShapeHull(convex);
				hull->buildHull(0.0);

				{
					//int strideInBytes = 9*sizeof(float);
					//int numVertices = hull->numVertices();
					//int numIndices =hull->numIndices();

					for (int t = 0; t < hull->numTriangles(); t++)
					{

						btVector3 triNormal;

						int index0 = hull->getIndexPointer()[t * 3 + 0];
						int index1 = hull->getIndexPointer()[t * 3 + 1];
						int index2 = hull->getIndexPointer()[t * 3 + 2];
						btVector3 pos0 = parentTransform*hull->getVertexPointer()[index0];
						btVector3 pos1 = parentTransform*hull->getVertexPointer()[index1];
						btVector3 pos2 = parentTransform*hull->getVertexPointer()[index2];
						triNormal = (pos1 - pos0).cross(pos2 - pos0);
						triNormal.normalize();

						for (int v = 0; v < 3; v++)
						{
							int index = hull->getIndexPointer()[t * 3 + v];
							btVector3 pos = parentTransform*hull->getVertexPointer()[index];
							indicesOut.push_back(vertexPositions.size());
							push_back(vertexPositions, pos);
							push_back(vertexNormals, triNormal);
						}
					}
				}
				delete hull;
			}
		}
		else
		{
			if (collisionShape->isCompound())
			{
				btCompoundShape* compound = (btCompoundShape*)collisionShape;
				for (int i = 0; i < compound->getNumChildShapes(); i++)
				{

					btTransform childWorldTrans = parentTransform * compound->getChildTransform(i);
					collisionShape2TriangleMesh(compound->getChildShape(i), childWorldTrans, vertexPositions, vertexNormals, indicesOut);
				}
			}
			else
			{
				ASSERT(false);
			}
		}
	}
	};
}

ConvexHullGameObject::ConvexHullGameObject(AUTORELEASE Object* obj)
	: m_fromVertices(false)
{
	setObject(obj);
}

ConvexHullGameObject::ConvexHullGameObject(GMfloat* vertices, GMuint count, Material& material)
	: m_vertices(vertices)
	, m_verticesCount(count)
	, m_material(material)
	, m_fromVertices(true)
{
}

btCollisionShape* ConvexHullGameObject::createCollisionShape()
{
	if (!m_fromVertices)
	{
		std::vector<Object::DataType> vertices;
		GMuint cnt = 0;
		BEGIN_FOREACH_OBJ(getObject(), childObj)
		{
			cnt += childObj->vertices().size();
		}
		END_FOREACH_OBJ;

		vertices.reserve(cnt);
		BEGIN_FOREACH_OBJ(getObject(), childObj)
		{
			for (auto iter = childObj->vertices().begin(); iter != childObj->vertices().end(); iter++)
			{
				vertices.push_back(*iter);
			}
		}
		END_FOREACH_OBJ;

		btConvexHullShape* shape = new btConvexHullShape(vertices.data(), cnt / 4, sizeof(Object::DataType) * 4);
		shape->optimizeConvexHull();
		return shape;
	}
	else
	{
		// 从vertices创建
		btConvexHullShape* shape = new btConvexHullShape(m_vertices, m_verticesCount);
		shape->optimizeConvexHull();
		return shape;
	}
}

void ConvexHullGameObject::getReadyForRender(DrawingList& list)
{
	D(d);
	btTransform trans = getRuntimeTransform();

	GMfloat glTrans[16];
	trans.getOpenGLMatrix(glTrans);

	vmath::mat4 M = getTransformMatrix(glTrans);
	const btVector3 scaling = d.collisionShape->getLocalScaling();
	vmath::mat4 S = vmath::scale(scaling[0], scaling[1], scaling[2]);

	if (d.animationState == Running)
	{
		AnimationMatrices mat = getAnimationMatrix();
		M = mat.tranlation * M * mat.rotation * mat.scaling;
	}

	DrawingItem item;
	memcpy(item.trans, M * S, sizeof(M));
	item.gameObject = this;
	list.push_back(item);
}

void ConvexHullGameObject::initPhysicsAfterCollisionObjectCreated()
{
	if (m_fromVertices)
		createTriangleMesh();

	RigidGameObject::initPhysicsAfterCollisionObjectCreated();
}

btTransform ConvexHullGameObject::getRuntimeTransform()
{
	D(d);
	btTransform trans;
	trans = d.transform;
	return trans;
}

void ConvexHullGameObject::createTriangleMesh()
{
	D(d);
	Object* obj = new Object();
	ChildObject* childObj = new ChildObject();

	ASSERT(d.collisionShape);
	btTransform trans;
	trans.setIdentity();

	std::vector<GMuint> indices;
	// 生成网格，形状的缩放、位置都会考虑在内
	collisionShape2TriangleMesh(d.collisionShape, trans, childObj->vertices(), childObj->normals(), indices);

	// 所有的Mesh，都采用同一材质
	Component* component = new Component();
	memcpy(&component->getMaterial(), &m_material, sizeof(Material));

	// 把每个面的顶点数（边数）传入Component
	for (GMuint i = 0; i < childObj->vertices().size() / 4; i++)
	{
		component->pushBackVertexOffset(3);
	}

	childObj->appendComponent(component);

	obj->append(childObj);
	setObject(obj);
}