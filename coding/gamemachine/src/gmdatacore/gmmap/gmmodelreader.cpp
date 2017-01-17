#include "stdafx.h"
#include "gmmodelreader.h"
#include "utilities/tinyxml/tinyxml.h"
#include "utilities/assert.h"
#include <map>
#include "gmdatacore/objreader/objreader.h"

typedef bool(*__Handler)(TiXmlElement&, GMModelReader*, OUT Object**);

static bool handleObject(TiXmlElement& elem, GMModelReader* reader, OUT Object** object)
{
	const char* type = elem.Attribute("type");

	if (strEqual(type, "obj"))
	{
		ObjReader objReader(reader->getFactory());
		std::string fullpath(reader->getWorkingDir());
		fullpath.append(elem.Attribute("path"));
		objReader.load(fullpath.c_str(), object);
	}

	return true;
}

struct __ModelReader_Handlers
{
	__ModelReader_Handlers()
	{
		__map["object"] = handleObject;
	}

	std::map<std::string, __Handler> __map;
};

static __Handler& getFunc(const char* name)
{
	static __ModelReader_Handlers rootFuncs;
	return rootFuncs.__map[name];
}

void read(TiXmlDocument& doc, GMModelReader* reader, OUT Object** object)
{
	TiXmlElement* root = doc.RootElement();
	for (TiXmlElement* item = root->FirstChildElement(); item; item = item->NextSiblingElement())
	{
		bool b = getFunc(item->Value())(*item, reader, object);
		ASSERT(b);
	}
}

GMModelReader::GMModelReader(IFactory* factory, const char* workingDir)
	: m_workingDir(workingDir)
	, m_factory(factory)
{

}

void GMModelReader::loadModel(const char* filename, OUT Object** object)
{
	TiXmlDocument doc;
	bool b = doc.LoadFile(filename);
	ASSERT(b);
	read(doc, this, object);
}