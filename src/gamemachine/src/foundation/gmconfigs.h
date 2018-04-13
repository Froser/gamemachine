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
	CalculateBSPFace_Bool = 0,
	DrawPolygonsAsLine_Bool,
	DrawSkyOnly_Bool,
	DrawPolygonNormalMode_I32,
	DrawLightmapOnly_Bool,
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

class GMConfig;
template <typename StateType>
struct GMConfigWrapper
{
	GMConfigWrapper() = default;
	GMConfigWrapper(const GMConfig& vm, const GMString guids[]) : m_vm(&vm), m_guids(guids) {}
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

typedef GMConfigWrapper<GMDebugConfigs> GMDebugConfig;
typedef GMConfigWrapper<GMRenderConfigs> GMRenderConfig;

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
