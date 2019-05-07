#ifndef __GMRENDERTECHNIQUE_H__
#define __GMRENDERTECHNIQUE_H__
#include <gmcommon.h>
BEGIN_NS

typedef GMlong GMRenderTechniqueID;
class GMRenderTechnique;
class GMRenderTechniques;

GM_PRIVATE_OBJECT(GMRenderTechniqueManager)
{
	const IRenderContext* context = nullptr;
	GMAtomic<GMRenderTechniqueID> id;
	Set<GMRenderTechniques> renderTechniques;
	Map<GMRenderTechniqueID, GMOwnedPtr<IShaderProgram>> shaderPrograms;
	bool inited = false;
};

class GM_EXPORT GMRenderTechniqueManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMRenderTechniqueManager)

public:
	enum
	{
		StartupTechinqueID = 10000,
	};

public:
	GMRenderTechniqueManager(const IRenderContext* context);

public:
	GMRenderTechniqueID addRenderTechniques(GMRenderTechniques renderTechniques);
	IShaderProgram* getShaderProgram(GMRenderTechniqueID id);
	void init();
	bool isEmpty();

public:
	inline const Set<GMRenderTechniques>& getRenderTechniques() const GM_NOEXCEPT
	{
		D(d);
		return d->renderTechniques;
	}
};


GM_PRIVATE_OBJECT(GMRenderTechnique)
{
	GMShaderType shaderType;
	GMString code[static_cast<GMsize_t>(GMRenderEnvironment::EndOfRenderEnvironment)];
	GMString path[static_cast<GMsize_t>(GMRenderEnvironment::EndOfRenderEnvironment)];
	GMString prefetch[static_cast<GMsize_t>(GMRenderEnvironment::EndOfRenderEnvironment)];
	GMRenderTechniques* parent = nullptr;
};

//! 表示一种渲染技术的类，它可以让GMGameObject在渲染时，调用到指定的着色器代码，完成预预设流程不一样的渲染。
class GM_EXPORT GMRenderTechnique : public GMObject
{
	GM_DECLARE_PRIVATE(GMRenderTechnique)
	GM_ALLOW_COPY_MOVE(GMRenderTechnique)
	GM_DECLARE_PROPERTY(ShaderType, shaderType)
	GM_FRIEND_CLASS(GMRenderTechniqueManager)
	GM_FRIEND_CLASS(GMRenderTechniques)

public:
	GMRenderTechnique(GMShaderType shaderType);

public:
	inline const GMString& getCode(GMRenderEnvironment type) const GM_NOEXCEPT
	{
		D(d);
		return d->code[static_cast<GMsize_t>(type)];
	}

	inline const GMString& getPath(GMRenderEnvironment type) const GM_NOEXCEPT
	{
		D(d);
		return d->path[static_cast<GMsize_t>(type)];
	}

	inline const GMString& getPrefetch(GMRenderEnvironment type) const GM_NOEXCEPT
	{
		D(d);
		return d->prefetch[static_cast<GMsize_t>(type)];
	}

public:
	void setCode(GMRenderEnvironment type, GMString code);
	void setPath(GMRenderEnvironment type, GMString path);
	void setPrefetch(GMRenderEnvironment type, GMString prefetch);

friend_methods(GMRenderTechniqueManager):
	void setId(GMRenderTechniqueID id);

friend_methods(GMRenderTechniques):
	inline void setParent(GMRenderTechniques* parent) GM_NOEXCEPT
	{
		D(d);
		d->parent = parent;
	}
};

GM_PRIVATE_OBJECT(GMRenderTechniques)
{
	GMRenderTechniqueID id;
	Vector<GMRenderTechnique> techniques;
};

class GM_EXPORT GMRenderTechniques : public GMObject
{
	GM_DECLARE_PRIVATE(GMRenderTechniques)
	GM_DECLARE_PROPERTY(Id, id)
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