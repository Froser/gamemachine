#ifndef __GMMODELREADER_MD5_H__
#define __GMMODELREADER_MD5_H__
#include <gmcommon.h>
#include "gmmodelreader.h"

#define BEGIN_DECLARE_MD5_HANDLER(tag, reader, scanner, cast) \
namespace {\
struct Handler_##tag : IMd5MeshHandler										\
{																			\
	virtual bool canHandle(const GMString& t) override						\
	{																		\
		return t == #tag;													\
	}																		\
	virtual bool handle(GMModelReader_MD5* _reader, GMScanner& scanner) {	\
		cast reader = gm_cast<cast>(_reader);

#define END_DECLARE_MD5_HANDLER() }}; }

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
	static GMQuat parseQuatFromVector3(GMScanner& s);
	static GMfloat calcQuatWFromVector3(GMVec3& v3);
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
	GMQuat orientation;
	GMString annotation;
};

typedef GMSkeletonVertex GMModelReader_MD5Mesh_Vertex;
typedef GMSkeletonWeight GMModelReader_MD5Mesh_Weight;
typedef GMSkeletonMesh GMModelReader_MD5Mesh_Mesh;

GM_ALIGNED_STRUCT(GMModelReader_MD5Anim_Joint)
{
	GMString jointName;
	GMint parentIndex;
	GMint flags;
	GMint startIndex;
	GMString annotation;
};

GM_ALIGNED_STRUCT(GMModelReader_MD5Anim_Hierarchy)
{
	Vector<GMModelReader_MD5Anim_Joint> joints;
};

GM_ALIGNED_STRUCT(GMModelReader_MD5Anim_Bound)
{
	GMVec3 boundMin;
	GMVec3 boundMax;
};

typedef GMBaseFrame GMModelReader_MD5Anim_Baseframe;

GM_ALIGNED_STRUCT(GMModelReader_MD5Anim_Frame)
{
	Vector<GMfloat> frameData;
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
	virtual bool load(const GMModelLoadSettings& settings, GMBuffer& buffer, REF GMAsset& asset) override;
	virtual bool test(const GMBuffer& buffer) override;
	virtual Vector<GMOwnedPtr<IMd5MeshHandler>>& getHandlers();

public:
	IMd5MeshHandler* findHandler(const GMString& tag);

public:
	static GMString removeQuotes(const GMString& string);
};

END_NS
#endif