#ifndef __GMCONFIG_H__
#define __GMCONFIG_H__
#include "defines.h"
#include "gmobject.h"
#include <gmstring.h>
#include "gmvariant.h"
BEGIN_NS

// 关于内部（如调试）的一些配置
struct GMStates_DebugOptions
{
	enum // DRAW_NORMAL
	{
		DRAW_NORMAL_OFF,
		DRAW_NORMAL_EYESPACE,
		DRAW_NORMAL_CAMERASPACE,
		DRAW_NORMAL_END
	};
};

struct GMStates_RenderOptions
{
	enum
	{
		AUTO_SAMPLE_OFFSET = -1,
	};

	struct GMEffects_ns
	{
		enum GMEffects
		{
			None = 0x00000000,
			Inversion = 0x00000001,
			Sharpen = 0x00000002,
			Blur = 0x00000004,
			Grayscale = 0x00000008,
			EdgeDetect = 0x00000010,

			LastEffectsTag,
			EndOfEffects = (LastEffectsTag - 1) << 2
		};
	};
};

enum class GMDebugConfigs
{
	DrawPolygonsAsLine_Bool,
	DrawLightmapOnly_Bool,
	DrawPolygonNormalMode_I32,
	RunProfile_Bool,

	FrameBufferPositionX_I32,
	FrameBufferPositionY_I32,
	FrameBufferHeight_I32,
	FrameBufferWidth_I32,
	FrameBufferIndex_I32,
	MAX,
};

enum class GMRenderConfigs
{
	RenderMode_I32,
	Effects_I32,
	BLUR_SAMPLE_OFFSET_X,
	BLUR_SAMPLE_OFFSET_Y,

	MAX,
};

using GMEffects = GMStates_RenderOptions::GMEffects_ns::GMEffects;

// GUIDs
namespace gm_config_guids
{
	const GMString DebugGUIDs[(GMuint)GMDebugConfigs::MAX] = {
		L"24D13A91-FB21-446e-A61D-565568586D0F",
		L"40A50C3C-BD6D-4474-B2CF-64B47079841E",
		L"FCFDB8C5-5D5D-45e4-B109-CF7B5A6A52A7",
		L"07029E4E-C12D-4970-B730-5D3CFB288F56",
		L"28618206-1BBC-42da-BE76-D231FDFB75F7",
		L"18DFA30F-0754-4c5f-BAD2-7F73B2861594",
		L"15C7030D-EB76-403e-A232-8615A0DBFCEB",
		L"2AD9D409-6878-4eca-AFEB-2B391F79C964",
		L"9D067D16-80E5-442b-B129-2F33AFC9BC9B",
	};

	const GMString RenderGUIDs[(GMuint)GMRenderConfigs::MAX] = {
		L"4ED48523-309E-40e0-87B6-36CEC8E58283",
		L"12471560-175E-4eff-B3F2-0569CA14E53D",
		L"3D2D42B5-9622-4a25-8FF0-E997AC1070C9",
		L"B6B6F7DA-B059-4cde-B317-1462F5EC2DCB",
	};
}

class GMConfig;
template <typename StateType>
struct GMConfigWrapperBase
{
	GMConfigWrapperBase() = default;
	GMConfigWrapperBase(const GMConfig& vm, const GMString guids[]) : m_vm(&vm), m_guids(guids) {}
	const GMVariant& get(StateType state) const
	{
		return const_cast<GMConfig&>(*m_vm)[m_guids[(GMuint)state]];
	}

	void set(StateType state, const GMVariant& variant)
	{
		const_cast<GMConfig&>(*m_vm)[m_guids[(GMuint)state]] = variant;
	}

	bool isEmpty() { return !!m_vm; }

private:
	const GMConfig* m_vm = nullptr;
	const GMString* m_guids = nullptr;
};

template <typename StateType>
struct GMConfigWrapper : public GMConfigWrapperBase<StateType> {};

#define GM_DEFINE_CONFIG(Enum, GUID, Alias)						\
template<>														\
struct GMConfigWrapper<Enum> : public GMConfigWrapperBase<Enum>	\
{																\
	GMConfigWrapper() = default;								\
	GMConfigWrapper(const GMConfig& vm)							\
		: GMConfigWrapperBase(vm, GUID)							\
	{}															\
};																\
typedef GMConfigWrapper<Enum> Alias;

GM_DEFINE_CONFIG(GMDebugConfigs, gm_config_guids::DebugGUIDs, GMDebugConfig);
GM_DEFINE_CONFIG(GMRenderConfigs, gm_config_guids::RenderGUIDs, GMRenderConfig);

class GMConfig : public HashMap<GMString, GMVariant, GMStringHashFunctor>
{
	typedef HashMap<GMString, GMVariant, GMStringHashFunctor> Base;

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

GM_PRIVATE_OBJECT(GMConfigs)
{
	HashMap<GMString, GMConfig, GMStringHashFunctor> configs;
};

class GMConfigs : public GMObject
{
	DECLARE_PRIVATE(GMConfigs)

	enum Category
	{
		Debug,
		Render,
	};

public:
	GMConfigs();

public:
	GMConfig& getConfig(Category state);
	const GMConfig& getConfig(Category state) const;
	
private:
	void init();
};

END_NS
#endif
