#include "stdafx.h"
#include "gmeffectgameobject.h"

BEGIN_NS

template<class T>
struct Nullable
{
	T value;
	bool isNull;

	Nullable() : isNull(true) {}
	Nullable(const T& val) : value(val), isNull(false) {}
	Nullable& operator=(T&& val) { isNull = false; value = std::forward<T>(val); return *this; }
	operator T&() { return value; }
	operator const T&() const { return value; }
};

GM_PRIVATE_OBJECT_UNALIGNED(GMEffectGameObject)
{
	GMRenderTechniqueID techId = 0;
	HashMap<GMString, GMVariant, GMStringHashFunctor> uniforms;
	HashMap<GMString, GMint32, GMStringHashFunctor> indicesCache;

	Nullable<GMint32> findIndexByUniformName(IShaderProgram* shaderProgram, const GMString& name);
};

Nullable<GMint32> GM_PRIVATE_NAME(GMEffectGameObject)::findIndexByUniformName(IShaderProgram* shaderProgram, const GMString& name)
{
	auto iter = indicesCache.find(name);
	if (iter == indicesCache.end())
	{
		// 没有从缓存中找到，那么从着色器程序中找
		GMint32 result = shaderProgram->getIndex(name);
		if (result != -1)
		{
			indicesCache[name] = result;
			return result;
		}
		else
		{
			return Nullable<GMint32>();
		}
	}

	return iter->second;
}

GMEffectGameObject::GMEffectGameObject(GMRenderTechniqueID techId, GMAsset asset)
	: GMGameObject(asset)
{
	GM_CREATE_DATA();
	D(d);
	d->techId = techId;
}

GMEffectGameObject::~GMEffectGameObject()
{

}

GMVariant GMEffectGameObject::getUniform(const GMString& name)
{
	D(d);
	auto iter = d->uniforms.find(name);
	if (iter == d->uniforms.end())
		return GMVariant();

	return iter->second;
}

void GMEffectGameObject::setUniform(const GMString& name, GMVariant value)
{
	D(d);
	d->uniforms[name] = value;
}

void GMEffectGameObject::onAppendingObjectToWorld()
{
	foreachModel([this](GMModel* model) {
		D(d);
		model->setTechniqueId(d->techId);
		model->setType(GMModelType::Custom);
	});
	Base::onAppendingObjectToWorld();
}

void GMEffectGameObject::onRenderShader(GMModel*, IShaderProgram* shaderProgram) const
{
	D(d);
	GM_ASSERT(shaderProgram);

	for (auto& uniformPair : d->uniforms)
	{
		Nullable<GMint32> index = d->findIndexByUniformName(shaderProgram, uniformPair.first);
		if (index.isNull) // 不存在这个变量
			continue;

		const GMVariant& uniform = uniformPair.second;
		if (uniform.isBool())
		{
			shaderProgram->setBool(index, uniform.toBool());
		}
		else if (uniform.isInt())
		{
			shaderProgram->setInt(index, uniform.toInt());
		}
		else if (uniform.isFloat())
		{
			shaderProgram->setFloat(index, uniform.toFloat());
		}
		else if (uniform.isMat4())
		{
			shaderProgram->setMatrix4(index, uniform.toMat4());
		}
		else if (uniform.isVec3())
		{
			shaderProgram->setVec3(index, ValuePointer(uniform.toVec3()));
		}
		else if (uniform.isVec4())
		{
			GMFloat4 f4;
			uniform.toVec4().loadFloat4(f4);
			shaderProgram->setVec4(index, f4);
		}
		else
		{
			gm_warning(gm_dbg_wrap("This variant type is not supported."));
		}
	}
}

END_NS
