#include "stdafx.h"
#include "gmrendertechnique.h"
#include "foundation/utilities/tools.h"
#include "foundation/gamemachine.h"

BEGIN_NS
GM_PRIVATE_OBJECT_UNALIGNED(GMRenderTechniqueManager)
{
	const IRenderContext* context = nullptr;
	GMAtomic<GMRenderTechniqueID> id;
	Set<GMRenderTechniques> renderTechniques;
	Map<GMRenderTechniqueID, GMOwnedPtr<IShaderProgram>> shaderPrograms;
	bool inited = false;
};

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

const Set<GMRenderTechniques>& GMRenderTechniqueManager::getRenderTechniques() const GM_NOEXCEPT
{
	D(d);
	return d->renderTechniques;
}

GMRenderTechniqueManager::GMRenderTechniqueManager(const IRenderContext* context)
{
	GM_CREATE_DATA();

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

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT_UNALIGNED(GMRenderTechnique)
{
	GMShaderType shaderType;
	GMString code[static_cast<GMsize_t>(GMRenderEnvironment::EndOfRenderEnvironment)];
	GMString path[static_cast<GMsize_t>(GMRenderEnvironment::EndOfRenderEnvironment)];
	GMString prefetch[static_cast<GMsize_t>(GMRenderEnvironment::EndOfRenderEnvironment)];
	GMRenderTechniques* parent = nullptr;
};

GM_DEFINE_PROPERTY(GMRenderTechnique, GMShaderType, ShaderType, shaderType)
GMRenderTechnique::GMRenderTechnique(GMShaderType shaderType)
{
	GM_CREATE_DATA();
	setShaderType(shaderType);
}

GMRenderTechnique::~GMRenderTechnique()
{

}

const GMString& GMRenderTechnique::getCode(GMRenderEnvironment type) const GM_NOEXCEPT
{
	D(d);
	return d->code[static_cast<GMsize_t>(type)];
}

GMRenderTechnique::GMRenderTechnique(const GMRenderTechnique& rhs)
{
	*this = rhs;
}

GMRenderTechnique::GMRenderTechnique(GMRenderTechnique&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMRenderTechnique& GMRenderTechnique::operator=(GMRenderTechnique&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}

GMRenderTechnique& GMRenderTechnique::operator=(const GMRenderTechnique& rhs)
{
	GM_COPY(rhs);
	return *this;
}

const GMString& GMRenderTechnique::getPath(GMRenderEnvironment type) const GM_NOEXCEPT
{
	D(d);
	return d->path[static_cast<GMsize_t>(type)];
}

const GMString& GMRenderTechnique::getPrefetch(GMRenderEnvironment type) const GM_NOEXCEPT
{
	D(d);
	return d->prefetch[static_cast<GMsize_t>(type)];
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

void GMRenderTechnique::setPrefetch(GMRenderEnvironment type, GMString prefetch)
{
	D(d);
	d->prefetch[static_cast<GMsize_t>(type)] = std::move(prefetch);
}

void GMRenderTechnique::setParent(GMRenderTechniques* parent) GM_NOEXCEPT
{
	D(d);
	d->parent = parent;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMRenderTechniques)
{
	GMRenderTechniqueID id;
	Vector<GMRenderTechnique> techniques;
};

GM_DEFINE_PROPERTY(GMRenderTechniques, GMRenderTechniqueID, Id, id)
GMRenderTechniques::GMRenderTechniques()
{
	GM_CREATE_DATA();
}

GMRenderTechniques::~GMRenderTechniques()
{

}

GMRenderTechniques::GMRenderTechniques(const GMRenderTechniques& rhs)
{
	*this = rhs;
}

GMRenderTechniques::GMRenderTechniques(GMRenderTechniques&& rhs) GM_NOEXCEPT
{
	*this = std::move(rhs);
}

GMRenderTechniques& GMRenderTechniques::operator=(const GMRenderTechniques& rhs)
{
	GM_COPY(rhs);
	return *this;
}

GMRenderTechniques& GMRenderTechniques::operator=(GMRenderTechniques&& rhs) GM_NOEXCEPT
{
	GM_MOVE(rhs);
	return *this;
}

void GMRenderTechniques::addRenderTechnique(GMRenderTechnique technique)
{
	D(d);
	technique.setParent(this);
	d->techniques.push_back(std::move(technique));
}

const Vector<GMRenderTechnique>& GMRenderTechniques::getTechniques() const GM_NOEXCEPT
{
	D(d);
	return d->techniques;
}

END_NS