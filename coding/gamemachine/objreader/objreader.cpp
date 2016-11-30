#include "stdafx.h"
#include "objreader.h"
#include "utilities/path.h"
#include "core/objstruct.h"

ObjReader::ObjReader(Mode mode)
{
	dataRef().setMode(mode);
}

void ObjReader::load(const char* filename, Object* obj)
{
	dataRef().setWorkingDir(Path::directoryName(filename));
	dataRef().beginLoad();
	std::ifstream file;
	file.open(filename, std::ios::in);
	if (file.good())
	{
		parse(file);
	}
	file.close();
	dataRef().endLoad();

	dataRef().getObject(obj);
}

void ObjReader::parse(std::ifstream& file)
{
	char line[LINE_MAX];
	while (file.getline(line, LINE_MAX))
	{
		dataRef().parseLine(line);
	}
}