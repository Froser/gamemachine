#include "stdafx.h"
#include "gmglgamepackagehandler.h"
#include "gmglshaders.h"
#include "gmglgraphic_engine.h"
#include "gmengine/controllers/gamemachine.h"
#include <string>
#include "utilities/path.h"

DefaultGMGLGamePackageHandler::DefaultGMGLGamePackageHandler(GamePackage* pk)
	: m_pk(pk)
{

}

GMbyte* DefaultGMGLGamePackageHandler::readFileFromPath(const char* path)
{
	return nullptr;
}

void DefaultGMGLGamePackageHandler::init()
{
	GamePackageData& d = m_pk->gamePackageData();
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
		
		std::string vert = d.packagePath + shaderMap[i] + ".vert",
			frag = d.packagePath + shaderMap[i] + ".vert";
		GMGLShaderInfo shadersInfo[] = {
			{ GL_VERTEX_SHADER, vert.c_str() },
			{ GL_FRAGMENT_SHADER, frag.c_str() },
		};

		shaders->appendShader(shadersInfo[0]);
		shaders->appendShader(shadersInfo[1]);
		engine->registerShader(ChildObject::NormalObject, shaders);
	}
}

GMbyte* DefaultGMGLGamePackageHandler::readFileFromPath(GMGLGamePackageFileType type, const char* path)
{
	return nullptr;
}