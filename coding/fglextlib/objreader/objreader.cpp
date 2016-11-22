#include "stdafx.h"
#include "objreader.h"
#include "utilities/path.h"

ObjReader::ObjReader(Mode mode)
{
	dataRef().setMode(mode);
}

void ObjReader::load(const char* filename)
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
}

void ObjReader::draw()
{
	dataRef().draw();
}

void ObjReader::parse(std::ifstream& file)
{
	char line[LINE_MAX];
	while (file.getline(line, LINE_MAX))
	{
		dataRef().parseLine(line);
	}
}