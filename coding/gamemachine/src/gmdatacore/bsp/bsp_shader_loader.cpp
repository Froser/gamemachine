#include "stdafx.h"
#include "bsp_shader_loader.h"
#include <vector>
#include <string>
#include "utilities/path.h"

BSPShaderLoader::BSPShaderLoader(const char* directory, BSPGameWorld& world)
	: m_world(world)
	, m_directory(directory)
{
}

void BSPShaderLoader::parseAll()
{
	std::vector<std::string> files = Path::getAllFiles(m_directory.c_str());
}