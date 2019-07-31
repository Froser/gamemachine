#include "stdafx.h"
#include "gmeffectreader.h"
#include "foundation/gamemachine.h"
#include <gmxml.h>
#include "gmengine/gmrendertechnique.h"
#include "gmengine/gameobjects/gmeffectgameobject.h"

BEGIN_NS

GM_INTERFACE(IEffectReader)
{
	virtual IEffectObjectFactory* createGameFactory(const GMString effectName, const IRenderContext* context, GMXMLElement* root) = 0;
};

class GMEffectReader_1_0 : public IEffectReader
{
public:
	virtual IEffectObjectFactory* createGameFactory(const GMString effectName, const IRenderContext* context, GMXMLElement* root) override;

private:
	GMRenderTechniqueID parseShader(GMXMLElement* shader);

private:
	GMString m_effectName;
	const IRenderContext* m_context = nullptr;
};

class EffectObjectFactoryImpl : public IEffectObjectFactory
{
public:
	EffectObjectFactoryImpl(GMRenderTechniqueID techId)
		: m_techId(techId)
	{
	}

public:
	virtual GMEffectGameObject* createGameObject(GMAsset asset) override
	{
		return new GMEffectGameObject(m_techId, asset);
	}

private:
	GMRenderTechniqueID m_techId;
};

IEffectObjectFactory* GMEffectReader_1_0::createGameFactory(const GMString effectName, const IRenderContext* context, GMXMLElement* root)
{
	m_context = context;
	m_effectName = effectName;
	GMRenderTechniqueID techId = parseShader(root->FirstChildElement("shader"));
	return new EffectObjectFactoryImpl(techId);
}

GMRenderTechniqueID GMEffectReader_1_0::parseShader(GMXMLElement* shader)
{
	if (!shader)
	{
		gm_warning(gm_dbg_wrap("No node 'shader' is detected."));
	}

	GMRenderTechniques techs;
	if (shader)
	{
		GMXMLElement* proc = shader->FirstChildElement(); // vs, ps, etc
		if (!proc)
			gm_warning(gm_dbg_wrap("There's no shader under element 'shader'."));

		GMShaderType shaderType = GMShaderType::Vertex;
		while (proc)
		{
			GMString name = proc->Name();
			if (name == "vs")
				shaderType = GMShaderType::Vertex;
			else if (name == "ps")
				shaderType = GMShaderType::Pixel;
			else if (name == "gs")
				shaderType = GMShaderType::Geometry;
			else
				gm_warning(gm_dbg_wrap("Cannot recognize shader name '{0}'. Use Vertex Shader(VS) instead."), name);

			GMRenderTechnique tech(shaderType);
			GMXMLElement* file = proc->FirstChildElement(); //file结点
			while (file)
			{
				GMRenderEnvironment env = GMRenderEnvironment::OpenGL;
				GMString targetStr = file->Attribute("target");
				if (targetStr == L"opengl")
					env = GMRenderEnvironment::OpenGL;
				else if (targetStr == L"directx11")
					env = GMRenderEnvironment::DirectX11;
				else
					gm_warning(gm_dbg_wrap("Cannot recognize target '{0}'. Use opengl instead."), targetStr);

				GMString fileName = file->GetText();
				GMBuffer fileBuf;
				GMString filepath;
				GM.getGamePackageManager()->readFile(GMPackageIndex::Effects, m_effectName + "/" + fileName, &fileBuf, &filepath);
				if (fileBuf.getSize() > 0)
				{
					fileBuf.convertToStringBuffer();
					tech.setCode(env, GMString((char*)fileBuf.getData()));
					tech.setPath(env, filepath);
				}
				else
				{
					gm_warning(gm_dbg_wrap("Cannot open file '{0}'. "), fileName);
				}

				file = file->NextSiblingElement();
			}

			proc = proc->NextSiblingElement();
			techs.addRenderTechnique(tech);
		}
	}

	return m_context->getEngine()->getRenderTechniqueManager()->addRenderTechniques(techs);
}

namespace
{
	IEffectReader* getEffectReaderByVersion(const GMString& version)
	{
		GMfloat v = GMString::parseFloat(version);
		if (v == 1.f)
		{
			return new GMEffectReader_1_0();
		}

		return nullptr;
	}
}

bool GMEffectReader::loadEffect(const GMString& effectName, const IRenderContext* context, IEffectObjectFactory** ppFactory)
{
	if (!ppFactory)
		return false;

	GMBuffer buffer;
	// 从manifest中读取基本效果配置
	GM.getGamePackageManager()->readFile(GMPackageIndex::Effects, effectName + L"/manifest.xml", &buffer);

	if (buffer.getSize() > 0)
	{
		buffer.convertToStringBuffer();

		GMXMLDocument doc;
		if (GMXMLError::XML_SUCCESS == doc.Parse((char*)buffer.getData()))
		{
			GMString version = doc.RootElement()->Attribute("version");
			IEffectReader* reader = getEffectReaderByVersion(version);
			if (reader)
			{
				*ppFactory = reader->createGameFactory(effectName, context, doc.RootElement());
				reader->destroy();
				return true;
			}
			else
			{
				gm_warning(gm_dbg_wrap("Cannot find reader of version '{0}'."), version);
			}
		}
		else
		{
			gm_warning(gm_dbg_wrap("Empty effect file or file doesn't exist."));
		}
	}
	else
	{
		gm_warning(gm_dbg_wrap("Empty effect file or file doesn't exist."));
	}
	return false;
}

END_NS