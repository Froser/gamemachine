#include "stdafx.h"
#include "gmrendertechnique.h"
#include "foundation/utilities/tools.h"

namespace
{
	static const GMString s_template_gl_code = L"if (GM_TechniqueId == ?0) { if (?1()) return; } \r\n";
	static const GMString s_template_gl_header = L"bool ?(void) { \r\n";
	static const GMString s_template_gl_footer = L"\r\n } \r\n";
}

GMRenderTechinqueID GMRenderTechniqueManager::addRenderTechnique(GMRenderTechniques renderTechniques)
{
	D(d);
	++d->id;
	renderTechniques.setId(d->id);
	d->renderTechniques.insert(std::move(renderTechniques));
	return d->id;
}

GMString GMRenderTechniqueManager::generateCode(GMShaderType type)
{
	D(d);
	// TODO onlyGL
	GMString code;
	for (const auto& techniques : d->renderTechniques)
	{
		for (const auto& technique : techniques.getTechniques())
		{
			if (technique.getShaderType() == type)
			{
				GMString l = s_template_gl_code.replace("?0", GMString(techniques.getId())).replace("?1", GMString(technique.getName()));
				code += l;
			}
		}
	}
	return GMConvertion::toCurrentEnvironmentString(code);
}

GMString GMRenderTechniqueManager::generateTechniques(GMShaderType type)
{
	D(d);
	//TODO onlyGL
	GMString code;
	for (const auto& techniques : d->renderTechniques)
	{
		for (const auto& technique : techniques.getTechniques())
		{
			if (technique.getShaderType() == type)
			{
				const GMString& userCode = technique.getCode(GMRenderTechniqueEngineType::OpenGL);
				if (userCode.isEmpty())
					code += s_template_gl_header.replace("?", GMString(technique.getName())) + L"return false;" + s_template_gl_footer;
				else
					code += userCode;
			}
		}
	}
	return GMConvertion::toCurrentEnvironmentString(code);
}

GMString GMRenderTechniqueManager::getInjectedCode(GMShaderType shaderType, REF GMString& source)
{
	GMStringReader reader(source);
	GMString line;
	GMString result;
	auto iter = reader.lineBegin();
	while (true)
	{
		line = *iter;

		// 寻找标记
		if (line.startsWith(L"/// {gm injection code}"))
			line += generateCode(shaderType);

		if (line.startsWith(L"/// {gm injection techniques}"))
			line += generateTechniques(shaderType);

		result += std::move(line);

		if (!iter.hasNextLine())
			break;

		++iter;
	}
	return result;
}

bool GMRenderTechniqueManager::isEmpty()
{
	D(d);
	return d->renderTechniques.empty();
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