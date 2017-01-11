#include "stdafx.h"
#include "objreader.h"
#include "utilities/path.h"
#include "gmdatacore/object.h"

ObjReader::ObjReader()
{
}

void ObjReader::load(const char* filename, OUT Object** obj)
{
	dataRef().setWorkingDir(Path::directoryName(filename));
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
	if (obj)
	{
		*obj = new Object();
		dataRef().setObject(*obj);
	}
	else
	{
		return;
	}

	char line[LINE_MAX];
	while (file.getline(line, LINE_MAX))
	{
		dataRef().parseLine(line);
	}
	dataRef().endParse();
}