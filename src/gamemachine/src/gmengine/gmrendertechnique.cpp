#include "stdafx.h"
#include "gmrendertechnique.h"
#include "foundation/utilities/tools.h"
#include "foundation/gamemachine.h"

GMRenderTechniqueID GMRenderTechniqueManager::addRenderTechniques(GMRenderTechniques renderTechniques)
{
	D(d);
	GM_ASSERT(!d->inited);
	if (d->inited)
	{
		gm_error(gm_dbg_wrap("addRenderTechniques must be called before GMRenderTechniqueManager::init"));
		return 0;
	}

	++d->id;
	renderTechniques.setId(d->id);
	d->renderTechniques.insert(std::move(renderTechniques));
	return d->id;
}

bool GMRenderTechniqueManager::isEmpty()
{
	D(d);
	return d->renderTechniques.empty();
}

GMRenderTechniqueManager::GMRenderTechniqueManager(const IRenderContext* context)
{
	D(d);
	d->id = StartupTechinqueID;
	d->context = context;
}

IShaderProgram* GMRenderTechniqueManager::getShaderProgram(GMRenderTechniqueID id)
{
	D(d);
	IShaderProgram* shaderProgram = d->shaderPrograms[id].get();
	GM_ASSERT(shaderProgram);
	return shaderProgram;
}

void GMRenderTechniqueManager::init()
{
	D(d);
	if (!d->inited)
	{
		IFactory* factory = GM.getFactory();
		d->shaderPrograms.clear();

		Vector<IShaderProgram*> out;
		factory->createShaderPrograms(d->context, *this, &out);

		GM_ASSERT(out.size() == getRenderTechniques().size());
		GMsize_t id = 0;
		for (auto& renderTechnique : getRenderTechniques())
		{
			auto& shaderProgramPtr = d->shaderPrograms[renderTechnique.getId()];
			shaderProgramPtr.reset(out[id++]);
		}
		d->inited = true;
	}
}

GMRenderTechnique::GMRenderTechnique(GMShaderType shaderType)
{
	setShaderType(shaderType);
}

void GMRenderTechnique::setCode(GMRenderEnvironment type, GMString code)
{
	D(d);
	d->code[static_cast<GMsize_t>(type)] = std::move(code);
}

void GMRenderTechnique::setPath(GMRenderEnvironment type, GMString path)
{
	D(d);
	d->path[static_cast<GMsize_t>(type)] = std::move(path);
}

void GMRenderTechniques::addRenderTechnique(GMRenderTechnique technique)
{
	D(d);
	technique.setParent(this);
	d->techniques.push_back(std::move(technique));
}