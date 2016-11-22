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
	std::ifstream file;
	file.open(filename, std::ios::in);
	parse(file);
	file.close();
}

void ObjReader::draw()
{
	dataRef().draw();
}

void ObjReader::parse(std::ifstream& file)
{
	while (!file.eof())
	{
		char line[LINE_MAX];
		file.getline(line, LINE_MAX);
		dataRef().parseLine(line);
	}
}