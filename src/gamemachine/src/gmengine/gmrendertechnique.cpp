#include "stdafx.h"
#include "gmrendertechnique.h"
#include "foundation/utilities/tools.h"
#include "foundation/gamemachine.h"

GMRenderTechinqueID GMRenderTechniqueManager::addRenderTechnique(GMRenderTechniques renderTechniques)
{
	D(d);
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
	d->context = context;
}

void GMRenderTechniqueManager::createShaderProgram(const GMRenderTechniques& renderTechniques, OUT IShaderProgram** out)
{
	D(d);
	IFactory* factory = GM.getFactory();
	factory->createShaderProgram(d->context, renderTechniques, out);
}

IShaderProgram* GMRenderTechniqueManager::getShaderProgram(GMRenderTechinqueID id)
{
	D(d);
	IShaderProgram* shaderProgram = d->shaderPrograms[id].get();
	if (!shaderProgram)
	{
		const GMRenderTechniques* techs = nullptr;
		for (auto& t : d->renderTechniques)
		{
			if (id == t.getId())
			{
				techs = &t;
				break;
			}
		}

		GM_ASSERT(techs);
		if (techs)
		{
			createShaderProgram(*techs, &shaderProgram);
			d->shaderPrograms[id] = GMOwnedPtr<IShaderProgram>(shaderProgram);
		}
	}

	GM_ASSERT(shaderProgram);
	return shaderProgram;
}

GMRenderTechnique::GMRenderTechnique(GMShaderType shaderType, GMString name)
{
	setShaderType(shaderType);
	setName(std::move(name));
}

void GMRenderTechnique::setCode(GMRenderTechniqueEngineType::Type type, GMString code)
{
	D(d);
	d->code[type] = std::move(code);
}

void GMRenderTechniques::addRenderTechnique(GMRenderTechnique technique)
{
	D(d);
	technique.setParent(this);
	d->techniques.push_back(std::move(technique));
}