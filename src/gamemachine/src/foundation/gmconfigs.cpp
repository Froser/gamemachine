#include "stdafx.h"
#include "interfaces.h"
#include "gmconfigs.h"
#include "assert.h"
#include "gmstring.h"

namespace
{
	const GMString StateGUIDs[] = {
		L"2E7152F9-53A3-4193-A14D-365736C9F9F6",
		L"5A7B099F-D7C0-4e10-A0B3-F032824EB7DD"
	};
}

GMDebugConfig GMConfig::asDebugConfig()
{
	return GMDebugConfig(*this);
}

GMRenderConfig GMConfig::asRenderConfig()
{
	return GMRenderConfig(*this);
}

const GMDebugConfig GMConfig::asDebugConfig() const
{
	return GMDebugConfig(*this);
}

const GMRenderConfig GMConfig::asRenderConfig() const
{
	return GMRenderConfig(*this);
}

GMConfigs::GMConfigs()
{
	init();
}

void GMConfigs::init()
{
	D(d);
	GMDebugConfig debugConfig = getConfig(GMConfigs::Debug).asDebugConfig();
	debugConfig.set(GMDebugConfigs::DrawPolygonsAsLine_Bool, false);
	debugConfig.set(GMDebugConfigs::DrawPolygonNormalMode, GMDrawPolygonNormalMode::Off);
	debugConfig.set(GMDebugConfigs::DrawLightmapOnly_Bool, false);
	debugConfig.set(GMDebugConfigs::RunProfile_Bool, false);
	debugConfig.set(GMDebugConfigs::FrameBufferPositionX_I32, 10);
	debugConfig.set(GMDebugConfigs::FrameBufferPositionY_I32, 10);
	debugConfig.set(GMDebugConfigs::FrameBufferWidth_I32, 250);
	debugConfig.set(GMDebugConfigs::FrameBufferHeight_I32, 250);
	debugConfig.set(GMDebugConfigs::FrameBufferIndex_I32, 0);

	GMRenderConfig renderConfig = getConfig(GMConfigs::Render).asRenderConfig();
	renderConfig.set(GMRenderConfigs::FilterMode, GMFilterMode::None);
	renderConfig.set(GMRenderConfigs::FilterKernelOffset_Vec2, GMVec2(1, 1));
	renderConfig.set(GMRenderConfigs::GammaCorrection_Bool, false);
	renderConfig.set(GMRenderConfigs::Gamma_Float, 2.2f);
	renderConfig.set(GMRenderConfigs::HDR_Bool, false);
	renderConfig.set(GMRenderConfigs::ToneMapping, GMToneMapping::Reinhard);
}

GMConfig& GMConfigs::getConfig(Category state)
{
	D(d);
	return d->configs[StateGUIDs[state]];
}

const GMConfig& GMConfigs::getConfig(Category state) const
{
	D(d);
	return d->configs[StateGUIDs[state]];
}