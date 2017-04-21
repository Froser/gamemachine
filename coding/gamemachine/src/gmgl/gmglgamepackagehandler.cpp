#include "stdafx.h"
#include "gmglgamepackagehandler.h"
#include "gmglshaders.h"
#include "gmglgraphic_engine.h"
#include "gmengine/controllers/gamemachine.h"
#include <string>
#include "utilities/path.h"
#include <fstream>
#include "gmengine/elements/bspgameworld.h"

#define PKD(d) GamePackageData& d = m_pk->gamePackageData();

DefaultGMGLGamePackageHandler::DefaultGMGLGamePackageHandler(GamePackage* pk)
	: m_pk(pk)
{

}

void DefaultGMGLGamePackageHandler::readFileFromPath(const char* path, OUT GMbyte** buffer)
{
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary);
	if (file.good())
	{
		file.seekg(0, std::ios::end);
		GMint size = file.tellg();
		*buffer = (GMbyte*)malloc(size + 1);
		file.seekg(0);
		file.read((char*)(*buffer), size + 1);
	}
	else
	{
		ASSERT(false);
	}
}

void DefaultGMGLGamePackageHandler::init()
{
	PKD(d);
	GMGLGraphicEngine* engine = static_cast<GMGLGraphicEngine*>(d.gameMachine->getGraphicEngine());

	// 装载所有shaders
	const char* shaderMap[] = 
	{
		"object",
		"sky"
	};

	for (GMint i = ChildObject::ObjectTypeBegin; i < ChildObject::ObjectTypeEnd; i++)
	{
		GMGLShaders* shaders = new GMGLShaders();
		
		std::string vert = pathRoot(PI_SHADERS) + shaderMap[i] + ".vert",
			frag = pathRoot(PI_SHADERS) + shaderMap[i] + ".frag";
		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, vert.c_str() },
			{ GL_FRAGMENT_SHADER, frag.c_str() },
		};

		shaders->appendShader(shadersInfo[0]);
		shaders->appendShader(shadersInfo[1]);
		shaders->load();
		engine->registerShader((ChildObject::ObjectType)i, shaders);
	}
}

std::string DefaultGMGLGamePackageHandler::pathRoot(PackageIndex index)
{
	PKD(d);

	switch (index)
	{
	case PI_MAPS:
		return d.packagePath + "maps/";
	case PI_SHADERS:
		return d.packagePath + "shaders/";
	case PI_TEXSHADERS:
		return d.packagePath + "texshaders/";
	case PI_TEXTURES:
		return d.packagePath + "textures/";
	default:
		ASSERT(false);
		break;
	}
	return "";
}