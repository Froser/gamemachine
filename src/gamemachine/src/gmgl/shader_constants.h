#ifndef __GMGL_SHADER_CONSTAINS_H__
#define __GMGL_SHADER_CONSTAINS_H__
#include <gmcommon.h>
#include <gmshader.h>
#include "foundation/assert.h"
#include <gmgraphicengine.h>

BEGIN_NS

#define		GMSHADER(memberName)				"GM_" memberName

//位置
#define		GMSHADER_SHADOW_MATRIX				GMSHADER("shadow_matrix")

//绘制调试
#define		GMSHADER_DEBUG_DRAW_NORMAL			GMSHADER("debug_draw_normal")

//特效
#define		GMSHADER_FRAMEBUFFER				GMSHADER("framebuffer")

// 获取名称辅助函数
// 字符串拼接太慢，因此采用一些预设的字符串

constexpr GMint32 GMGL_MAX_LIGHT_COUNT = 10; //灯光最大数量

inline const GMString& getTextureUniformName(GMTextureType t)
{
	static const GMString empty;
	switch (t)
	{
	case GMTextureType::Ambient:
		return GM_VariablesDesc.AmbientTextureName;
	case GMTextureType::Diffuse:
		return GM_VariablesDesc.DiffuseTextureName;
	case GMTextureType::Specular:
		return GM_VariablesDesc.SpecularTextureName;
	case GMTextureType::NormalMap:
		return GM_VariablesDesc.NormalMapTextureName;
	case GMTextureType::Lightmap:
		return GM_VariablesDesc.LightMapTextureName;
	case GMTextureType::Albedo:
		return GM_VariablesDesc.AlbedoTextureName;
	case GMTextureType::MetallicRoughnessAO:
		return GM_VariablesDesc.MetallicRoughnessAOTextureName;
	case GMTextureType::EndOfCommonTexture:
	default:
		GM_ASSERT(false);
		break;
	}
	return empty;
}

template <typename T>
inline GMsize_t verifyIndicesContainer(Vector<T>& container, IShaderProgram* shaderProgram)
{
	GMsize_t sz = static_cast<GMGLShaderProgram*>(shaderProgram)->getProgram();
	if (container.size() <= sz)
	{
		static T t = { 0 };
		container.resize(sz + 1, t);
	}
	return sz;
}

template <typename U>
inline GMsize_t verifyIndicesContainer(Vector<Vector<U>>& container, IShaderProgram* shaderProgram)
{
	GMsize_t sz = static_cast<GMGLShaderProgram*>(shaderProgram)->getProgram();
	if (container.size() <= sz)
	{
		container.resize(sz + 1);
	}
	return sz;
}

#define getVariableIndex(shaderProgram, index, name) (index ? index : (index = shaderProgram->getIndex(name)))

END_NS
#endif
