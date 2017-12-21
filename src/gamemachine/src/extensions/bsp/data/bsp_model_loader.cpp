#include "stdafx.h"
#include "bsp_model_loader.h"
#include "foundation/vector.h"
#include "gmdata/gamepackage/gmgamepackage.h"
#include "extensions/bsp/render/gmbspgameworld.h"
#include "foundation/utilities/tinyxml/tinyxml.h"
#include "foundation/utilities/tools.h"
#include "foundation/gamemachine.h"

BSPModelLoader::BSPModelLoader()
{
	D(d);
	d->world = nullptr;
}

BSPModelLoader::~BSPModelLoader()
{
	D(d);
	for (auto iter = d->modelDocs.begin(); iter != d->modelDocs.end(); iter++)
	{
		delete *iter;
	}

	for (auto iter = d->items.begin(); iter != d->items.end(); iter++)
	{
		if ((*iter).second)
			delete (*iter).second;
	}
}

void BSPModelLoader::init(const GMString& directory, GMBSPGameWorld* world)
{
	D(d);
	d->directory = directory;
	d->world = world;
}

void BSPModelLoader::load()
{
	D(d);
	GMGamePackage* pk = GameMachine::instance().getGamePackageManager();
	Vector<GMString> files = pk->getAllFiles(d->directory);

	for (auto& file : files)
	{
		GMBuffer buf;
		pk->readFileFromPath(file, &buf);
		buf.convertToStringBuffer();
		parse((const char*)buf.buffer);
	}
}

Model* BSPModelLoader::find(const GMString& classname)
{
	D(d);
	ModelMap::iterator iter;
	if ((iter = d->items.find(classname)) == d->items.end())
		return nullptr;

	return ((*iter).second);
}

void BSPModelLoader::parse(const char* buf)
{
	D(d);
	TiXmlDocument* doc = new TiXmlDocument();
	if (doc->Parse(buf) != 0)
	{
		gm_error("xml load error at %d: %s", doc->ErrorRow(), doc->ErrorDesc());
		delete doc;
		return;
	}

	if (doc->Error())
		return;

	d->modelDocs.push_back(doc);
	TiXmlElement* root = doc->RootElement();
	TiXmlElement* it = root->FirstChildElement();
	for (; it; it = it->NextSiblingElement())
	{
		TiXmlElement* elem = it;
		if (!strEqual(elem->Value(), "item"))
			gm_warning(_L("First node must be 'item'."));

		const char* name = elem->Attribute("name");
		for (TiXmlElement* it = root->FirstChildElement(); it; it = it->NextSiblingElement())
		{
			parseItem(it);
		}
	}
}

void BSPModelLoader::parseItem(TiXmlElement* ti)
{
	D(d);
	const char* classname = ti->Attribute("classname");
	if (!classname)
	{
		gm_error(_L("model file 'classname' attribute missing:"));
		return;
	}

	Model* m = new Model;
	m->model[0] = 0;
	strcpy_s(m->classname, classname);

	const char* b;
	if ( (b = ti->Attribute("create")))
	{
		Scanner s(b);
		GMint value;
		s.nextInt(&value);
		m->create = value != 0;
	}

	if ((b = ti->Attribute("extents")))
	{
		Scanner s(b);
		for (GMint i = 0; i < 3; i++)
		{
			s.nextFloat(&m->extents[i]);
		}
	}

	if ((b = ti->Attribute("model")))
	{
		strcpy_s(m->model, b);
	}

	d->items[m->classname] = m;
}