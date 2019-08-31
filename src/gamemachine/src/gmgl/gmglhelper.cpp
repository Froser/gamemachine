#include "stdafx.h"
#include "gmglhelper.h"
#include "gmglshaderprogram.h"
#include <GL/glew.h>

BEGIN_NS

namespace
{
	typedef HashMap<GMString, GMString, GMStringHashFunctor> DefinesMap;

	GMGLInfo s_glinfo;
	Vector<GMGLShaderInfo> s_defaultShaders;
	Vector<GMGLShaderInfo> s_defaultIncludes;

	bool parseShaderType(const char* type, GMShaderType st, GMXMLElement* e, Vector<GMGLShaderInfo>& infos, DefinesMap& definesMap)
	{
		e = e->FirstChildElement(type);
		if (e)
		{
			// 遍历所有define结点
			DefinesMap defines;
			GMXMLElement* defineElem = e->FirstChildElement("define");
			while (defineElem)
			{
				const char* macro = defineElem->Attribute("macro");
				if (macro)
					definesMap[macro] = defineElem->GetText();
				defineElem = defineElem->NextSiblingElement("define");
			}

			// 遍历所有file结点
			GMXMLElement* fileElem = e->FirstChildElement("file");
			while (fileElem)
			{
				GMGLShaderInfo info;
				info.type = GMGLShaderInfo::toGLShaderType(st);
				GMBuffer buffer;
				GM.getGamePackageManager()->readFile(GMPackageIndex::Shaders, fileElem->Attribute("src"), &buffer);
				if (buffer.getSize() == 0)
					return false;

				buffer.convertToStringBuffer();
				info.source = GMString((const char*)buffer.getData());
				infos.push_back(std::move(info));

				fileElem = fileElem->NextSiblingElement("file");
			}
		}
		return true;
	}

	void parseCustomIncludes(GMXMLElement* e)
	{
		e = e->FirstChildElement("includes");
		if (e)
		{
			e = e->FirstChildElement();
			while (e)
			{
				GMShaderType st = GMShaderType::Unknown;
				const char* type = e->Name();
				if (type)
				{
					GMString typeStr = type;
					if (typeStr == L"ps")
						st = GMShaderType::Pixel;
					else if (typeStr == L"vs")
						st = GMShaderType::Vertex;
					else
						gm_error(gm_dbg_wrap("Wrong shader type of includes: {0}"), typeStr);

					if (st != GMShaderType::Unknown)
					{
						// 遍历所有file结点
						GMXMLElement* fileElem = e->FirstChildElement("file");
						while (fileElem)
						{
							GMGLShaderInfo info;
							info.type = GMGLShaderInfo::toGLShaderType(st);
							GMBuffer buffer;
							GM.getGamePackageManager()->readFile(GMPackageIndex::Shaders, fileElem->Attribute("src"), &buffer);
							if (buffer.getSize() == 0)
								return;

							buffer.convertToStringBuffer();
							info.source = GMString((const char*)buffer.getData());
							s_defaultIncludes.push_back(std::move(info));

							fileElem = fileElem->NextSiblingElement("file");
						}
					}
				}
				e = e->NextSiblingElement();
			}
		}
	}
}

bool parseShaderTag(const IRenderContext* context, const char* tag, GMXMLElement* e, GameMachineInterfaceID interfaceID)
{
	s_defaultIncludes.clear();
	parseCustomIncludes(e);

	e = e->FirstChildElement(tag);
#if GM_RASPBERRYPI
	if (const char* condition = e->Attribute("condition"))
	{
		if (GMString::stringEquals(condition, "except_raspberrypi"))
			return true;
	}
#endif
	bool isDefault = false;
	const char* defaultStr = e->Attribute("default");
	if (defaultStr && GMString::stringEquals(defaultStr, "true"))
		isDefault = true;

	DefinesMap vsdm, psdm, gsdm;
	Vector<GMGLShaderInfo> shaderInfos;
	bool r = parseShaderType("vs", GMShaderType::Vertex, e, shaderInfos, vsdm);
	if (!r) return r;
	r = parseShaderType("ps", GMShaderType::Pixel, e, shaderInfos, psdm);
	if (!r) return r;
	r = parseShaderType("gs", GMShaderType::Geometry, e, shaderInfos, gsdm);
	if (!r) return r;

	if (isDefault)
		s_defaultShaders = shaderInfos;

	GMGLShaderProgram* prog = new GMGLShaderProgram(context);
	prog->setDefinesMap(GMShaderType::Vertex, vsdm);
	prog->setDefinesMap(GMShaderType::Pixel, psdm);
	prog->setDefinesMap(GMShaderType::Geometry, gsdm);

	for (auto& shaderInfo : shaderInfos)
	{
		prog->attachShader(shaderInfo);
	}

	r = prog->load();
	if (!r) return r;

	r = context->getEngine()->setInterface(interfaceID, prog);
	if (!r) return r;
	return true;
}

bool GMGLHelper::loadShader(const IRenderContext* context, const GMString& manifest)
{
	GMXMLDocument doc;
	GMXMLError err = doc.Parse(manifest.toStdString().c_str());
	if (err == GMXMLError::XML_SUCCESS)
	{
		bool b = parseShaderTag(context, "forward", doc.RootElement(), GameMachineInterfaceID::GLForwardShaderProgram);
		if (!b) return b;
		b = parseShaderTag(context, "filter", doc.RootElement(), GameMachineInterfaceID::GLFiltersShaderProgram);
		if (!b) return b;
		b = parseShaderTag(context, "deferredgeometry", doc.RootElement(), GameMachineInterfaceID::GLDeferredShaderGeometryProgram);
		if (!b) return b;
		b = parseShaderTag(context, "deferredlight", doc.RootElement(), GameMachineInterfaceID::GLDeferredShaderLightProgram);
		if (!b) return b;
	}
	else
	{
		gm_error(gm_dbg_wrap("Error in parse shader manifest {0}, {1}"), doc.ErrorName(), doc.ErrorStr());
	}
	return true;
}

Vector<GMGLShaderInfo> GMGLHelper::getDefaultShaderCodes(GMShaderType st)
{
	Vector<GMGLShaderInfo> results;
	for (const auto& s : s_defaultShaders)
	{
		if (s.type == GMGLShaderInfo::toGLShaderType(st))
			results.push_back(s);
	}
	return results;
}

Vector<GMGLShaderInfo> GMGLHelper::getDefaultShaderIncludes(GMShaderType st)
{
	Vector<GMGLShaderInfo> results;
	for (const auto& s : s_defaultIncludes)
	{
		if (s.type == GMGLShaderInfo::toGLShaderType(st))
			results.push_back(s);
	}
	return results;
}

void GMGLHelper::initOpenGL()
{
	static std::once_flag flag;
	std::call_once(flag, [](GMGLInfo& info){
		if (const char* lpcszName = (const char*) glGetString(GL_VENDOR))
			info.vendor = lpcszName;
		if (const char* lpcszName = (const char*) glGetString(GL_RENDERER))
			info.renderer = lpcszName;
		if (const char* lpcszName = (const char*) glGetString(GL_VERSION))
			info.version = lpcszName;
		if (const char* lpcszName = (const char*) glGetString(GL_EXTENSIONS))
			info.extensions = lpcszName;
		if (const char* lpcszName = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION))
			info.shadingLanguageVersions = lpcszName;
	}, s_glinfo);
}

const GMGLInfo& getOpenGLInfo()
{
	return s_glinfo;
}

bool GMGLHelper::isOpenGLShaderLanguageES()
{
	static std::once_flag flag;
	static bool s_result;
	std::call_once(flag, [](bool& result){
#if GM_RASPBERRYPI
		result = true;
#else
		std::wstring shaderLanguages = s_glinfo.shadingLanguageVersions.toStdWString();
		result = shaderLanguages.find(L"ES") != std::wstring::npos;
#endif
	}, s_result);
	return s_result;
}

bool GMGLHelper::isSupportGeometryShader()
{
#if GM_RASPBERRYPI
	return false;
#else
	return true;
#endif
}

END_NS
