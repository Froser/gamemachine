#include "stdafx.h"
#include <string>
#include <map>
#include "utilities/tinyxml/tinyxml.h"
#include "gmmapreader.h"
#include "gmmap.h"
#include "utilities/path.h"
#include "utilities/assert.h"
#include "utilities/scanner.h"

#define SAFE_SSCANF(in, format, out)	\
{										\
	const char* _str = in;				\
	if (_str)							\
		sscanf_s(_str, format, out);	\
}

typedef bool (*__Handler)(TiXmlElement&, GMMap*);

bool handleMeta(TiXmlElement& elem, GMMap* map)
{
	map->meta.author = elem.Attribute("author");
	map->meta.name = elem.Attribute("name");
	return true;
}

bool handleTextures(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		GMMapTexture texture = { 0 };
		SAFE_SSCANF(child->Attribute("id"), "%i", &texture.id);
		texture.path = child->Attribute("path");
		texture.type = GMMapTexture::getType(child->Attribute("type"));
		map->textures.push_back(texture);
	}

	return true;
}

bool handleObjects(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		GMMapObject object = { 0 };
		SAFE_SSCANF(child->Attribute("id"), "%i", &object.id);
		object.path = child->Attribute("path");
		object.type = GMMapObject::getType(child->Attribute("type"));
		map->objects.push_back(object);
	}

	return true;
}

bool handleMaterals(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		GMMapMaterial material = { 0 };
		SAFE_SSCANF(child->Attribute("id"), "%i", &material.id);
		
		{
			Scanner scanner(child->Attribute("ka"));
			scanner.nextFloat(&material.material.Ka[0]);
			scanner.nextFloat(&material.material.Ka[1]);
			scanner.nextFloat(&material.material.Ka[2]);
		}

		{
			Scanner scanner(child->Attribute("kd"));
			scanner.nextFloat(&material.material.Kd[0]);
			scanner.nextFloat(&material.material.Kd[1]);
			scanner.nextFloat(&material.material.Kd[2]);
		}

		{
			Scanner scanner(child->Attribute("ks"));
			scanner.nextFloat(&material.material.Ks[0]);
			scanner.nextFloat(&material.material.Ks[1]);
			scanner.nextFloat(&material.material.Ks[2]);
		}

		{
			Scanner scanner(child->Attribute("ke"));
			scanner.nextFloat(&material.material.Ke[0]);
			scanner.nextFloat(&material.material.Ke[1]);
			scanner.nextFloat(&material.material.Ke[2]);
		}

		const char* shininess = child->Attribute("shininess");
		if (shininess)
			SAFE_SSCANF(shininess, "%f", &material.material.shininess);

		map->materials.push_back(material);
	}
	return true;
}

bool handleEntities(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		GMMapEntity entity = { 0 };
		SAFE_SSCANF(child->Attribute("id"), "%i", &entity.id);
		SAFE_SSCANF(child->Attribute("objref"), "%i", &entity.objRef);
		SAFE_SSCANF(child->Attribute("textureref_0"), "%i", &entity.textureRef_0);
		SAFE_SSCANF(child->Attribute("textureref_1"), "%i", &entity.textureRef_1);
		SAFE_SSCANF(child->Attribute("textureref_2"), "%i", &entity.textureRef_2);
		SAFE_SSCANF(child->Attribute("textureref_3"), "%i", &entity.textureRef_3);
		SAFE_SSCANF(child->Attribute("textureref_4"), "%i", &entity.textureRef_4);
		SAFE_SSCANF(child->Attribute("textureref_5"), "%i", &entity.textureRef_5);
		SAFE_SSCANF(child->Attribute("materialref_0"), "%i", &entity.materialRef_0);
		SAFE_SSCANF(child->Attribute("materialref_1"), "%i", &entity.materialRef_1);
		SAFE_SSCANF(child->Attribute("materialref_2"), "%i", &entity.materialRef_2);
		SAFE_SSCANF(child->Attribute("materialref_3"), "%i", &entity.materialRef_3);
		SAFE_SSCANF(child->Attribute("materialref_4"), "%i", &entity.materialRef_4);
		SAFE_SSCANF(child->Attribute("materialref_5"), "%i", &entity.materialRef_5);
		map->entities.push_back(entity);
	}

	return true;
}

bool handleInstances(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		GMMapInstance instance = { 0 };
		SAFE_SSCANF(child->Attribute("id"), "%f", &instance.id);
		SAFE_SSCANF(child->Attribute("x"), "%f", &instance.x);
		SAFE_SSCANF(child->Attribute("y"), "%f", &instance.y);
		SAFE_SSCANF(child->Attribute("z"), "%f", &instance.z);
		SAFE_SSCANF(child->Attribute("radius"), "%f", &instance.radius);
		SAFE_SSCANF(child->Attribute("scale"), "%f", &instance.scale);
		SAFE_SSCANF(child->Attribute("mass"), "%f", &instance.mass);
		map->instances.push_back(instance);
	}

	return true;
}

struct __rootFuncs
{
	__rootFuncs()
	{
		__map["meta"] = handleMeta;
		__map["textures"] = handleTextures;
		__map["objects"] = handleObjects;
		__map["materials"] = handleMaterals;
		__map["entities"] = handleEntities;
		__map["instances"] = handleInstances;
	}

	std::map<std::string, __Handler> __map;
};

__Handler& getFunc(const char* name)
{
	static __rootFuncs rootFuncs;
	return rootFuncs.__map[name];
}

void readHead(TiXmlDocument& doc, GMMap* map)
{
	TiXmlElement* root = doc.RootElement();
	for (TiXmlElement* item = root->FirstChildElement(); item; item = item->NextSiblingElement())
	{
		bool b = getFunc(item->Value())(*item, map);
		ASSERT(b);
	}
}

void GMMapReader::ReadGMM(const char* filename, OUT GMMap** map)
{
	TiXmlDocument doc;
	bool b = doc.LoadFile(filename);
	ASSERT(b);

	GMMap* gmmMap;
	if (map)
	{
		gmmMap = new GMMap;
		*map = gmmMap;

		std::string currentPath = Path::directoryName(filename);
		gmmMap->workingDir = currentPath;
	}

	readHead(doc, gmmMap);
}