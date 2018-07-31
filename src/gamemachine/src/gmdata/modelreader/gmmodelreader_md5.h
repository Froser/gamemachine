#ifndef __GMMODELREADER_MD5_H__
#define __GMMODELREADER_MD5_H__
#include <gmcommon.h>
#include "gmmodelreader.h"

#define BEGIN_DECLARE_MD5_HANDLER(tag, reader, scanner, cast) \
struct Handler_##tag : IMd5MeshHandler										\
{																			\
	virtual bool canHandle(const GMString& t) override						\
	{																		\
		return t == #tag;													\
	}																		\
	virtual bool handle(GMModelReader_MD5* _reader, GMScanner& scanner) {	\
		cast reader = gm_cast<cast>(_reader);

#define END_DECLARE_MD5_HANDLER() }};

#define NEW_MD5_HANDLER(tag) GMOwnedPtr<IMd5MeshHandler>(new Handler_##tag())

BEGIN_NS

namespace
{
	template <typename CharType>
	class StringReader
	{
	public:
		StringReader::StringReader(const CharType* string)
			: m_p(string)
		{
		}

		bool StringReader::readLine(GMString& line)
		{
			line.clear();
			GMint offset = 0;
			while (isNotReturn(*m_p))
			{
				if (!*m_p)
					return false;
				line += *(m_p++);
			}

			m_p++;
			return true;
		}

	private:
		inline bool slashPredicate(GMwchar c)
		{
			return c == L'/';
		}

		inline bool isNotReturn(char p)
		{
			return p != '\r' && p != '\n';
		}

		inline bool isNotReturn(GMwchar p)
		{
			return p != L'\r' && p != L'\n';
		}

	private:
		const CharType* m_p;
	};
}

struct GMMD5VectorParser
{
	static GMVec3 parseVector3(GMScanner& s);
	static GMVec2 parseVector2(GMScanner& s);
};

class GMModelReader_MD5;
class GMScanner;

GM_INTERFACE(IMd5MeshHandler)
{
	virtual bool canHandle(const GMString& tag) = 0;
	virtual bool handle(GMModelReader_MD5* reader, GMScanner& scanner) = 0;
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

GM_PRIVATE_OBJECT(GMModelReader_MD5)
{
	Vector<GMOwnedPtr<IMd5MeshHandler>> handlers;
	GMint MD5Version;
	GMString meshFile;
	GMString animFile;
};

class GMModelReader_MD5 : public GMObject, public IModelReader
{
	GM_DECLARE_PRIVATE(GMModelReader_MD5)
	GM_DECLARE_PROPERTY(MD5Version, MD5Version, GMint)
	GM_DECLARE_PROPERTY(MeshFile, meshFile, GMString)
	GM_DECLARE_PROPERTY(AnimFile, animFile, GMString)

public:
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, OUT GMModels** models) override;
	virtual bool test(const GMBuffer& buffer) override;
	virtual Vector<GMOwnedPtr<IMd5MeshHandler>>& getHandlers();

public:
	IMd5MeshHandler* findHandler(const GMString& tag);
};

END_NS
#endif