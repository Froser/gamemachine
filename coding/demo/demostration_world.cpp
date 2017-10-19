#include "stdafx.h"
#include "demostration_world.h"
#include <gmgl.h>

void DemostrationWorld::init(GameHandlers& handlers)
{
	// ´´½¨Demo²Ëµ¥
	D(d);

	for (auto& handler : d->handlers)
	{
		addMenu(handler);
	}
}

void DemostrationWorld::addMenu(GameHandlerItem& item)
{
	D(d);
	gm::GMImage2DGameObject* entrance = new gm::GMImage2DGameObject();
	entrance->attachEvent(*entrance, gm::GM_CONTROL_EVENT_ENUM(MouseDown), mouseDownCallback);
	// entrance->setText("...");
}

void DemostrationWorld::mouseDownCallback(GMObject*, GMObject*)
{

}

//////////////////////////////////////////////////////////////////////////
void DemostrationEntrance::init()
{
	gm::GMGLGraphicEngine* engine = static_cast<gm::GMGLGraphicEngine*> (GameMachine::instance().getGraphicEngine());
	engine->setShaderLoadCallback(this);
	GMSetRenderState(RENDER_MODE, GMStates_RenderOptions::DEFERRED);
	//GMSetRenderState(EFFECTS, GMEffects::Grayscale);
	GMSetRenderState(RESOLUTION_X, 800);
	GMSetRenderState(RESOLUTION_Y, 600);

	GMGamePackage* pk = GameMachine::instance().getGamePackageManager();
#ifdef _DEBUG
	pk->loadPackage("D:/gmpk");
#else
	pk->loadPackage((GMPath::getCurrentPath() + _L("gm.pk0")));
#endif
}

void DemostrationEntrance::start()
{
}

void DemostrationEntrance::event(GameMachineEvent evt)
{

}

void DemostrationEntrance::onLoadForwardShader(const GMMeshType type, gm::GMGLShaderProgram& shader)
{
	GMBuffer vertBuf, fragBuf;
	GMString vertPath, fragPath;
	switch (type)
	{
	case GMMeshType::Model3D:
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "model3d.vert", &vertBuf, &vertPath);
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "model3d.frag", &fragBuf, &fragPath);
		break;
	case GMMeshType::Model2D:
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "model2d.vert", &vertBuf, &vertPath);
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "model2d.frag", &fragBuf, &fragPath);
		break;
	case GMMeshType::Particles:
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "particles.vert", &vertBuf, &vertPath);
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "particles.frag", &fragBuf, &fragPath);
		break;
	default:
		break;
	}

	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	shader.attachShader(shadersInfo[0]);
	shader.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadDeferredPassShader(gm::GMGLDeferredRenderState state, gm::GMGLShaderProgram& shaderProgram)
{
	GMBuffer vertBuf, fragBuf;
	GMString vertPath, fragPath;
	switch (state)
	{
	case gm::GMGLDeferredRenderState::PassingGeometry:
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "deferred/geometry_pass.vert", &vertBuf, &vertPath);
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "deferred/geometry_pass.frag", &fragBuf, &fragPath);
		break;
	case gm::GMGLDeferredRenderState::PassingMaterial:
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "deferred/material_pass.vert", &vertBuf, &vertPath);
		GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "deferred/material_pass.frag", &fragBuf, &fragPath);
		break;
	default:
		break;
	}
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	shaderProgram.attachShader(shadersInfo[0]);
	shaderProgram.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadDeferredLightPassShader(gm::GMGLShaderProgram& lightPassProgram)
{
	GMBuffer vertBuf, fragBuf;
	GMString vertPath, fragPath;
	GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "deferred/light_pass.vert", &vertBuf, &vertPath);
	GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "deferred/light_pass.frag", &fragBuf, &fragPath);
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	lightPassProgram.attachShader(shadersInfo[0]);
	lightPassProgram.attachShader(shadersInfo[1]);
}

void DemostrationEntrance::onLoadEffectsShader(gm::GMGLShaderProgram& lightPassProgram)
{
	GMBuffer vertBuf, fragBuf;
	GMString vertPath, fragPath;
	GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "effects/effects.vert", &vertBuf, &vertPath);
	GameMachine::instance().getGamePackageManager()->readFile(GMPackageIndex::Shaders, "effects/effects.frag", &fragBuf, &fragPath);
	vertBuf.convertToStringBuffer();
	fragBuf.convertToStringBuffer();

	GMGLShaderInfo shadersInfo[] = {
		{ GL_VERTEX_SHADER, (const char*)vertBuf.buffer, vertPath },
		{ GL_FRAGMENT_SHADER, (const char*)fragBuf.buffer, fragPath },
	};

	lightPassProgram.attachShader(shadersInfo[0]);
	lightPassProgram.attachShader(shadersInfo[1]);
}
