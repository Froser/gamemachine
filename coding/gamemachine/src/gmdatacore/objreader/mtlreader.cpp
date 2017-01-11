#include "stdafx.h"
#include "mtlreader.h"
#include "utilities/path.h"
#include <string>

MtlReader::MtlReader()
{

}

void MtlReader::load(const char* filename)
{
	D(d);
	d.setWorkingDir(Path::directoryName(filename));
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
	D(d);
	LOG_ASSERT_MSG(d.getMaterials().find(std::string(name)) != d.getMaterials().end(), "warning: material" + std::string(name) + " not found.");

	return (d.getMaterials())[std::string(name)];
}

void MtlReader::parse(std::ifstream& file)
{
	D(d);
	char line[LINE_MAX];
	while (file.getline(line, LINE_MAX))
	{
		d.parseLine(line);
	}
}