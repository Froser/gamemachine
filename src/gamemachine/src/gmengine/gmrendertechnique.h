#ifndef __GMRENDERTECHNIQUE_H__
#define __GMRENDERTECHNIQUE_H__
#include <gmcommon.h>
BEGIN_NS

typedef GMlong GMRenderTechinqueID;
class GMRenderTechnique;
class GMRenderTechniques;

struct GMRenderTechniqueEngineType
{
	enum Type
	{
		OpenGL,
		DirectX11,
		EndOfRenderTechniqueEngineType
	};
};

GM_PRIVATE_OBJECT(GMRenderTechniqueManager)
{
	const IRenderContext* context = nullptr;
	GMAtomic<GMRenderTechinqueID> id = 10000;
	Set<GMRenderTechniques> renderTechniques;
	Map<GMRenderTechinqueID, GMOwnedPtr<IShaderProgram>> shaderPrograms;
};

class GMRenderTechniqueManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMRenderTechniqueManager)

public:
	GMRenderTechniqueManager(const IRenderContext* context);

public:
	GMRenderTechinqueID addRenderTechnique(GMRenderTechniques renderTechniques);
	IShaderProgram* getShaderProgram(GMRenderTechinqueID id);
	bool isEmpty();

private:
	void createShaderProgram(const GMRenderTechniques& renderTechniques, OUT IShaderProgram** out);
};


GM_PRIVATE_OBJECT(GMRenderTechnique)
{
	GMShaderType shaderType;
	GMString code[GMRenderTechniqueEngineType::EndOfRenderTechniqueEngineType];
	GMRenderTechniques* parent = nullptr;
};

//! 表示一种渲染技术的类，它可以让GMGameObject在渲染时，调用到指定的着色器代码，完成预预设流程不一样的渲染。
class GMRenderTechnique : public GMObject
{
	GM_DECLARE_PRIVATE(GMRenderTechnique)
	GM_ALLOW_COPY_MOVE(GMRenderTechnique)
	GM_DECLARE_PROPERTY(ShaderType, shaderType, GMShaderType)
	GM_FRIEND_CLASS(GMRenderTechniqueManager)
	GM_FRIEND_CLASS(GMRenderTechniques)

public:
	GMRenderTechnique(GMShaderType shaderType);

public:
	inline const GMString& getCode(GMRenderTechniqueEngineType::Type type) const GM_NOEXCEPT
	{
		D(d);
		return d->code[type];
	}

public:
	void setCode(GMRenderTechniqueEngineType::Type type, GMString code);

friend_methods(GMRenderTechniqueManager):
	void setId(GMRenderTechinqueID id);

friend_methods(GMRenderTechniques):
	inline void setParent(GMRenderTechniques* parent) GM_NOEXCEPT
	{
		D(d);
		d->parent = parent;
	}
};

GM_PRIVATE_OBJECT(GMRenderTechniques)
{
	GMRenderTechinqueID id;
	Vector<GMRenderTechnique> techniques;
};

class GMRenderTechniques : public GMObject
{
	GM_DECLARE_PRIVATE(GMRenderTechniques)
	GM_DECLARE_PROPERTY(Id, id, GMRenderTechinqueID)
	GM_ALLOW_COPY_MOVE(GMRenderTechniques)

public:
	GMRenderTechniques() = default;

public:
	void addRenderTechnique(GMRenderTechnique technique);

public:
	inline const Vector<GMRenderTechnique>& getTechniques() const GM_NOEXCEPT
	{
		D(d);
		return d->techniques;
	}
};

inline bool operator < (const GMRenderTechniques& lhs, const GMRenderTechniques& rhs)
{
	return lhs.getId() < rhs.getId();
}

END_NS
#endif