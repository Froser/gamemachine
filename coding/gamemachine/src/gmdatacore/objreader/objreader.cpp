#include "stdafx.h"
#include "objreader.h"
#include "utilities/path.h"
#include "gmdatacore/object.h"

ObjReader::ObjReader(IFactory* factory)
{
	D(d);
	d.m_factory = factory;
}

void ObjReader::load(const char* filename, OUT Object** obj)
{
	D(d);
	d.setWorkingDir(Path::directoryName(filename));
	std::ifstream file;
	file.open(filename, std::ios::in);
	if (file.good())
	{
		parse(file, obj);
	}
	else
	{
		LOG_ASSERT_MSG(false, (std::string("Model loading error: ") + filename));
	}
	file.close();
}

void ObjReader::parse(std::ifstream& file, OUT Object** obj)
{
	D(d);
	if (obj)
	{
		*obj = new Object();
		d.setObject(*obj);
	}
	else
	{
		return;
	}

	char line[LINE_MAX];
	while (file.getline(line, LINE_MAX))
	{
		d.parseLine(line);
	}
	d.endParse();
}