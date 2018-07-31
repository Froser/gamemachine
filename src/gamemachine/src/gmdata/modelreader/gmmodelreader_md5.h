#ifndef __GMMODELREADER_MD5_H__
#define __GMMODELREADER_MD5_H__
#include <gmcommon.h>
BEGIN_NS

class GMModelReader_MD5Mesh;
class GMScanner;
GM_INTERFACE(IMd5MeshHandler)
{
	virtual bool canHandle(const GMString& tag) = 0;
	virtual bool handle(GMModelReader_MD5Mesh* reader, GMScanner& scanner) = 0;
};

GM_ALIGNED_STRUCT(GMModelReader_MD5Mesh_Joint)
{
	GMString name;
	GMint parentIndex;
	GMVec3 position;
	GMVec3 orientation;
	GMString annotation;
};

GM_ALIGNED_STRUCT(GMModelReader_MD5Mesh_Vertex)
{
	GMint vertexIndex;
	GMVec2 texCoords;
	GMint startWeight;
	GMint weightCount;
};

GM_ALIGNED_STRUCT(GMModelReader_MD5Mesh_Weight)
{
	GMint weightIndex;
	GMint jointIndex;
	GMfloat weightBias;
	GMVec3 weightPosition;
};

GM_ALIGNED_STRUCT(GMModelReader_MD5Mesh_Mesh)
{
	GMString shader;
	GMint numVertices;
	Vector<GMModelReader_MD5Mesh_Vertex> vertices;
	GMint numTriangles;
	Vector<GMVec4> triangleIndices;
	GMint numWeights;
	Vector<GMModelReader_MD5Mesh_Weight> weights;
};

class GMModelReader_MD5 : public GMObject, public IModelReader
{
public:
	virtual Vector<GMOwnedPtr<IMd5MeshHandler>>& getHandlers() = 0;

	IMd5MeshHandler* findHandler(const GMString& tag)
	{
		auto& handlers = getHandlers();
		for (decltype(auto) handler : handlers)
		{
			if (handler->canHandle(tag))
				return handler.get();
		}
		return nullptr;
	}
};

END_NS
#endif