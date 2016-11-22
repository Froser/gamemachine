#include "stdafx.h"
#include "mtlreader.h"

MtlReader::MtlReader()
{

}

void MtlReader::load(const char* filename)
{
	std::ifstream file;
	file.open(filename, std::ios::in);
	parse(file);
	file.close();
}

const MaterialProperties& MtlReader::getProperties(const char* name)
{
	return (dataRef().getMaterials())[std::string(name)];
}

void MtlReader::parse(std::ifstream& file)
{
	while (!file.eof())
	{
		char line[LINE_MAX];
		file.getline(line, LINE_MAX);
		dataRef().parseLine(line);
	}
}