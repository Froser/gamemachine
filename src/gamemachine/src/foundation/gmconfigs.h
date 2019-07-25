#ifndef __GMCONFIG_H__
#define __GMCONFIG_H__
#include "defines.h"
#include "gmobject.h"
#include <gmstring.h>
#include "gmvariant.h"
BEGIN_NS

// 关于内部（如调试）的一些配置
struct GMDrawPolygonNormalMode
{
	enum // DRAW_NORMAL
	{
		Off,
		WorldSpace,
		EyeSpace,
		EndOfEnum,
	};
};

struct GMFilterMode
{
	typedef GMint32 Mode;

	enum
	{
		None,
		Inversion,
		Sharpen,
		Blur,
		Grayscale,
		EdgeDetect,
		Blend,
		EndOfEnum,
	};
};

struct GMToneMapping
{
	typedef GMint32 Mode;

	enum
	{
		Reinhard
	};
};

enum class GMDebugConfigs
{
	WireFrameMode_Bool,
	DrawLightmapOnly_Bool,
	DrawPolygonNormalMode,
	RunProfile_Bool,

	FrameBufferPositionX_I32,
	FrameBufferPositionY_I32,
	FrameBufferHeight_I32,
	FrameBufferWidth_I32,
	FrameBufferIndex_I32,
	Max,
};

enum class GMRenderConfigs
{
	FilterMode,
	FilterKernelOffset_Vec2,
	GammaCorrection_Bool,
	Gamma_Float,
	HDR_Bool,
	ToneMapping,
	ViewCascade_Bool,
	BlendFactor_Vec3,
	Max,
};

class GMConfig;
template <typename StateType>
struct GMConfigWrapperBase
{
	GMConfigWrapperBase() = default;
	GMConfigWrapperBase(const GMConfig& vm) : m_vm(&vm) {}
	const GMVariant& get(StateType state) const;
	void set(StateType state, const GMVariant& variant);
	bool isEmpty() { return !m_vm; }
	void verify(StateType state) const;

private:
	const GMConfig* m_vm = nullptr;
};

template <typename StateType>
struct GM_EXPORT GMConfigWrapper : public GMConfigWrapperBase<StateType> {};

#define GM_DEFINE_CONFIG(Enum, Alias)							\
template<>														\
struct GM_EXPORT GMConfigWrapper<Enum> : public GMConfigWrapperBase<Enum>	\
{																\
	GMConfigWrapper() = default;								\
	GMConfigWrapper(const GMConfig& vm)							\
		: GMConfigWrapperBase(vm)								\
	{}															\
};																\
typedef GMConfigWrapper<Enum> Alias;

GM_DEFINE_CONFIG(GMDebugConfigs, GMDebugConfig);
GM_DEFINE_CONFIG(GMRenderConfigs, GMRenderConfig);

class GM_EXPORT GMConfig : public Vector<GMVariant>
{
	typedef Vector<GMVariant> Base;

public:
	using Base::Base;

public:
	GMDebugConfig asDebugConfig();
	GMRenderConfig asRenderConfig();
	const GMDebugConfig asDebugConfig() const;
	const GMRenderConfig asRenderConfig() const;

	template <typename T>
	T as()
	{
		return T(*this);
	}
};

GM_PRIVATE_CLASS(GMConfigs);
class GM_EXPORT GMConfigs : public GMObject
{
	GM_DECLARE_PRIVATE(GMConfigs)

public:
	enum Category
	{
		Debug,
		Render,
	};

public:
	GMConfigs();
	~GMConfigs();

public:
	GMConfig& getConfig(Category state);
	const GMConfig& getConfig(Category state) const;
	
private:
	void init();
	void verify(Category state) const;
};

template <typename StateType>
void GMConfigWrapperBase<StateType>::verify(StateType state) const
{
	GMConfig& vm = const_cast<GMConfig&>(*m_vm);
	GMuint32 s = (GMuint32)state;
	if (vm.size() <= s)
		vm.resize(s + 1);
}

template <typename StateType>
const GMVariant& GMConfigWrapperBase<StateType>::get(StateType state) const
{
	verify(state);
	return const_cast<GMConfig&>(*m_vm)[(GMuint32)state];
}

template <typename StateType>
void GMConfigWrapperBase<StateType>::set(StateType state, const GMVariant& variant)
{
	verify(state);
	const_cast<GMConfig&>(*m_vm)[GMuint32(state)] = variant;
}

END_NS
#endif
