#include "stdafx.h"
#include "mtlreader.h"
#include "utilities/path.h"

MtlReader::MtlReader()
{

}

void MtlReader::load(const char* filename)
{
	dataRef().setWorkingDir(Path::directoryName(filename));
	std::ifstream file;
	file.open(filename, std::ios::in);
	if (file.good())
	{
		parse(file);
	}
	file.close();
}

const MaterialProperties& MtlReader::getProperties(const char* name)
{
	return (dataRef().getMaterials())[std::string(name)];
}

void MtlReader::parse(std::ifstream& file)
{
	char line[LINE_MAX];
	while (file.getline(line, LINE_MAX))
	{
		dataRef().parseLine(line);
	}
}