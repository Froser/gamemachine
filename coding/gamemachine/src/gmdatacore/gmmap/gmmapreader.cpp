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
		map->textures.insert(texture);
	}

	return true;
}

bool handleObjects(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		GMMapObject object = { 0 };
		SAFE_SSCANF(child->Attribute("id"), "%i", &object.id);
		SAFE_SSCANF(child->Attribute("width"), "%f", &object.width);
		SAFE_SSCANF(child->Attribute("height"), "%f", &object.height);
		SAFE_SSCANF(child->Attribute("depth"), "%f", &object.depth);
		SAFE_SSCANF(child->Attribute("radius"), "%f", &object.radius);
		SAFE_SSCANF(child->Attribute("slices"), "%f", &object.slices);
		SAFE_SSCANF(child->Attribute("stacks"), "%f", &object.stacks);
		SAFE_SSCANF(child->Attribute("mag"), "%f", &object.magnification);
		object.path = child->Attribute("path");
		object.type = GMMapObject::getType(child->Attribute("type"));
		map->objects.insert(object);
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

		map->materials.insert(material);
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

		{
			const char* r = child->Attribute("textureref");
			if (r)
			{
				for (GMuint i = 0; i < GMMapEntity::MAX_REF; i++)
				{
					SAFE_SSCANF(r, "%i", &entity.textureRef[i]);
				}
			}

			for (GMuint i = 0; i < GMMapEntity::MAX_REF; i++)
			{
				char ref[32] = "textureref_";
				char str[32];
				sprintf(str, "%d", i + 1);
				strcat(ref, str);
				SAFE_SSCANF(child->Attribute(ref), "%i", &entity.textureRef[i]);
			}
		}

		{
			const char* r = child->Attribute("materialref");
			if (r)
			{
				for (GMuint i = 0; i < GMMapEntity::MAX_REF; i++)
				{
					SAFE_SSCANF(r, "%i", &entity.materialRef[i]);
				}
			}

			for (GMuint i = 0; i < GMMapEntity::MAX_REF; i++)
			{
				char ref[32] = "materialref_";
				char str[32];
				sprintf(str, "%d", i + 1);
				strcat_s(ref, str);
				SAFE_SSCANF(child->Attribute(ref), "%i", &entity.materialRef[i]);
			}
		}
		map->entities.insert(entity);
	}

	return true;
}

bool handleInstances(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		GMMapInstance instance = { 0 };
		SAFE_SSCANF(child->Attribute("id"), "%i", &instance.id);
		SAFE_SSCANF(child->Attribute("entityref"), "%i", &instance.entityRef);
		SAFE_SSCANF(child->Attribute("mass"), "%f", &instance.mass);

		{
			Scanner scanner(child->Attribute("position"));
			scanner.nextFloat(&instance.position[0]);
			scanner.nextFloat(&instance.position[1]);
			scanner.nextFloat(&instance.position[2]);
		}

		{
			Scanner scanner(child->Attribute("scale"));
			scanner.nextFloat(&instance.scale[0]);
			bool b = scanner.nextFloat(&instance.scale[1]);
			if (b)
			{
				scanner.nextFloat(&instance.scale[2]);
			}
			else
			{
				instance.scale[1] = instance.scale[0];
				instance.scale[2] = instance.scale[0];
			}

			if (instance.scale[0] == 0 || instance.scale[1] == 0 || instance.scale[2] == 0)
				instance.scale[0] = instance.scale[1] = instance.scale[2] = 1;
		}

		{
			const char* rotation = child->Attribute("rotation");
			if (rotation)
			{
				Scanner scanner(rotation);
				scanner.nextFloat(&instance.rotation[0]);
				scanner.nextFloat(&instance.rotation[1]);
				scanner.nextFloat(&instance.rotation[2]);
				scanner.nextFloat(&instance.rotation[3]);
			}
			else
			{
				instance.rotation[0] = 1;
				instance.rotation[1] = 0;
				instance.rotation[2] = 0;
				instance.rotation[3] = 0;
			}
		}

		map->instances.insert(instance);
	}

	return true;
}

bool handleLights(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		GMMapLight light = { 0 };
		SAFE_SSCANF(child->Attribute("id"), "%i", &light.id);
		SAFE_SSCANF(child->Attribute("range"), "%f", &light.range);
		SAFE_SSCANF(child->Attribute("shadow"), "%i", &light.shadow);
		light.type = GMMapLight::getType(child->Attribute("type"));

		{
			Scanner scanner(child->Attribute("position"));
			scanner.nextFloat(&light.position[0]);
			scanner.nextFloat(&light.position[1]);
			scanner.nextFloat(&light.position[2]);
		}

		{
			Scanner scanner(child->Attribute("rgb"));
			scanner.nextFloat(&light.rgb[0]);
			scanner.nextFloat(&light.rgb[1]);
			scanner.nextFloat(&light.rgb[2]);
		}

		map->lights.insert(light);
	}

	return true;
}

bool handleSettings(TiXmlElement& elem, GMMap* map)
{
	for (TiXmlElement* child = elem.FirstChildElement(); child; child = child->NextSiblingElement())
	{
		if (strEqual(child->Value(), "gravity"))
		{
			Scanner scanner (child->Attribute("vector"));
			scanner.nextFloat(&map->settings.gravity.vector[0]);
			scanner.nextFloat(&map->settings.gravity.vector[1]);
			scanner.nextFloat(&map->settings.gravity.vector[2]);
		}
		else if (strEqual(child->Value(), "character"))
		{
			{
				Scanner scanner(child->Attribute("position"));
				scanner.nextFloat(&map->settings.character.position[0]);
				scanner.nextFloat(&map->settings.character.position[1]);
				scanner.nextFloat(&map->settings.character.position[2]);
			}

			{
				Scanner scanner(child->Attribute("jumpspeed"));
				scanner.nextFloat(&map->settings.character.jumpSpeed[0]);
				scanner.nextFloat(&map->settings.character.jumpSpeed[1]);
				scanner.nextFloat(&map->settings.character.jumpSpeed[2]);
			}

			SAFE_SSCANF(child->Attribute("radius"), "%f", &map->settings.character.radius);
			SAFE_SSCANF(child->Attribute("height"), "%f", &map->settings.character.height);
			SAFE_SSCANF(child->Attribute("stepheight"), "%f", &map->settings.character.stepHeight);
			SAFE_SSCANF(child->Attribute("fallspeed"), "%f", &map->settings.character.fallSpeed);
			SAFE_SSCANF(child->Attribute("freemove"), "%i", &map->settings.character.freemove);
		}
	}
	return true;
}

struct __Handlers
{
	__Handlers()
	{
		__map["meta"] = handleMeta;
		__map["textures"] = handleTextures;
		__map["objects"] = handleObjects;
		__map["materials"] = handleMaterals;
		__map["entities"] = handleEntities;
		__map["instances"] = handleInstances;
		__map["lights"] = handleLights;
		__map["settings"] = handleSettings;
	}

	std::map<std::string, __Handler> __map;
};

__Handler& getFunc(const char* name)
{
	static __Handlers rootFuncs;
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

void GMMapReader::readGMM(const char* filename, OUT GMMap** map)
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