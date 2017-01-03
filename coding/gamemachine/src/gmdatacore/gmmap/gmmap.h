#ifndef __GMMAP_H__
#define __GMMAP_H__
#include "common.h"
#include <string>
#include <vector>
#include "gmdatacore/object.h"

BEGIN_NS

typedef GMuint ID;

class GMMapString : public std::string
{
public:
	GMMapString& operator = (const char* str)
	{
		if (str)
			this->std::string::operator = (str);
		return *this;
	}
};

struct GMMapMeta
{
	GMMapString author;
	GMMapString name;
};

struct GMMapTexture
{
	enum GMMapTexturesType
	{
		Error = -1,
		Ambient,
	};

	static GMMapTexturesType getType(const char* name);

	ID id;
	GMMapTexturesType type;
	GMMapString path;
};

struct GMMapObject
{
	enum GMMapObjectType
	{
		Error = -1,
		FromFile,
		Cube,
		Sphere,
		Sky,
	};

	static GMMapObjectType getType(const char* name);

	ID id;
	GMMapObjectType type;
	GMMapString path;
};

struct GMMapMaterial
{
	ID id;
	Material material;
};

struct GMMapEntity
{
	ID id;
	ID objRef;
	ID materialRef_0;
	ID materialRef_1;
	ID materialRef_2;
	ID materialRef_3;
	ID materialRef_4;
	ID materialRef_5;
	ID textureRef_0;
	ID textureRef_1;
	ID textureRef_2;
	ID textureRef_3;
	ID textureRef_4;
	ID textureRef_5;
};

struct GMMapInstance
{
	ID id;
	ID entityRef;
	GMfloat x, y, z, radius, scale;
	GMfloat mass;
};

struct GMMap
{
	GMMapMeta meta;
	std::vector<GMMapTexture> textures;
	std::vector<GMMapObject> objects;
	std::vector<GMMapMaterial> materials;
	std::vector<GMMapEntity> entities;
	std::vector<GMMapInstance> instances;
	std::string workingDir;
};

END_NS
#endif