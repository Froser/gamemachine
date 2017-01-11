#include "stdafx.h"
#include "mtlreader.h"
#include "utilities/path.h"
#include <string>

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
	else
	{
		LOG_ASSERT_MSG(false, std::string(filename) + " load error.");
	}
	file.close();
}

const MaterialProperties& MtlReader::getProperties(const char* name)
{
	LOG_ASSERT_MSG(dataRef().getMaterials().find(std::string(name)) != dataRef().getMaterials().end(), "warning: material" + std::string(name) + " not found.");

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