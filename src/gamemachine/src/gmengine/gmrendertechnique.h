#ifndef __GMRENDERTECHNIQUE_H__
#define __GMRENDERTECHNIQUE_H__
#include <gmcommon.h>
BEGIN_NS

typedef GMlong GMRenderTechniqueID;
class GMRenderTechnique;
class GMRenderTechniques;

GM_PRIVATE_CLASS(GMRenderTechniqueManager);
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
	const Set<GMRenderTechniques>& getRenderTechniques() const GM_NOEXCEPT;
};


GM_PRIVATE_CLASS(GMRenderTechnique);
//! 表示一种渲染技术的类，它可以让GMGameObject在渲染时，调用到指定的着色器代码，完成预预设流程不一样的渲染。
class GM_EXPORT GMRenderTechnique
{
	GM_DECLARE_PRIVATE(GMRenderTechnique)
	GM_DECLARE_PROPERTY(GMShaderType, ShaderType)
	GM_FRIEND_CLASS(GMRenderTechniqueManager)
	GM_FRIEND_CLASS(GMRenderTechniques)

public:
	GMRenderTechnique(GMShaderType shaderType);
	GMRenderTechnique(const GMRenderTechnique& rhs);
	GMRenderTechnique(GMRenderTechnique&& rhs) GM_NOEXCEPT;
	GMRenderTechnique& operator=(const GMRenderTechnique& rhs);
	GMRenderTechnique& operator=(GMRenderTechnique&& rhs) GM_NOEXCEPT;

public:
	const GMString& getCode(GMRenderEnvironment type) const GM_NOEXCEPT;
	const GMString& getPath(GMRenderEnvironment type) const GM_NOEXCEPT;
	const GMString& getPrefetch(GMRenderEnvironment type) const GM_NOEXCEPT;

public:
	void setCode(GMRenderEnvironment type, GMString code);
	void setPath(GMRenderEnvironment type, GMString path);
	void setPrefetch(GMRenderEnvironment type, GMString prefetch);

friend_methods(GMRenderTechniqueManager):
	void setId(GMRenderTechniqueID id);

friend_methods(GMRenderTechniques):
	void setParent(GMRenderTechniques* parent) GM_NOEXCEPT;
};

GM_PRIVATE_CLASS(GMRenderTechniques);
class GM_EXPORT GMRenderTechniques
{
	GM_DECLARE_PRIVATE(GMRenderTechniques)
	GM_DECLARE_PROPERTY(GMRenderTechniqueID, Id)

public:
	GMRenderTechniques();
	~GMRenderTechniques();
	GMRenderTechniques(const GMRenderTechniques& rhs);
	GMRenderTechniques(GMRenderTechniques&& rhs) GM_NOEXCEPT;
	GMRenderTechniques& operator=(const GMRenderTechniques& rhs);
	GMRenderTechniques& operator=(GMRenderTechniques&& rhs) GM_NOEXCEPT;

public:
	void addRenderTechnique(GMRenderTechnique technique);

public:
	const Vector<GMRenderTechnique>& getTechniques() const GM_NOEXCEPT;
};

inline bool operator < (const GMRenderTechniques& lhs, const GMRenderTechniques& rhs)
{
	return lhs.getId() < rhs.getId();
}

END_NS
#endif