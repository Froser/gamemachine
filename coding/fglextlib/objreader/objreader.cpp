#include "stdafx.h"
#include "objreader.h"

ObjReader::ObjReader()
{

}

void ObjReader::load(const char* filename)
{
	std::ifstream file;
	file.open(filename, std::ios::in);
	parse(file);
	file.close();
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