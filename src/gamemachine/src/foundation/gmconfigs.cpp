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

	const GMString DebugGUIDs[(GMuint)GMDebugConfigs::MAX] = {
		L"24D13A91-FB21-446e-A61D-565568586D0F",
		L"40A50C3C-BD6D-4474-B2CF-64B47079841E",
		L"FCFDB8C5-5D5D-45e4-B109-CF7B5A6A52A7",
		L"07029E4E-C12D-4970-B730-5D3CFB288F56",
		L"28618206-1BBC-42da-BE76-D231FDFB75F7",
		L"18DFA30F-0754-4c5f-BAD2-7F73B2861594",
		L"15C7030D-EB76-403e-A232-8615A0DBFCEB",
		L"2AD9D409-6878-4eca-AFEB-2B391F79C964",
		L"2C046FEB-E530-45e1-9F53-A5EE7E2A4EEB",
		L"02C6B265-A7D6-4a5b-8ACC-31C8E1522A6F",
		L"9D067D16-80E5-442b-B129-2F33AFC9BC9B",
	};

	const GMString RenderGUIDs[(GMuint)GMRenderConfigs::MAX] = {
		L"4ED48523-309E-40e0-87B6-36CEC8E58283",
		L"12471560-175E-4eff-B3F2-0569CA14E53D",
		L"3D2D42B5-9622-4a25-8FF0-E997AC1070C9",
		L"B6B6F7DA-B059-4cde-B317-1462F5EC2DCB",
	};
}

GMDebugConfig GMConfig::asDebugConfig()
{
	return GMDebugConfig(*this, DebugGUIDs);
}

GMRenderConfig GMConfig::asRenderConfig()
{
	return GMRenderConfig(*this, RenderGUIDs);
}

const GMDebugConfig GMConfig::asDebugConfig() const
{
	return GMDebugConfig(*this, DebugGUIDs);
}

const GMRenderConfig GMConfig::asRenderConfig() const
{
	return GMRenderConfig(*this, RenderGUIDs);
}

GMConfigs::GMConfigs()
{
	init();
}

void GMConfigs::init()
{
	D(d);
	GMConfigWrapper<GMDebugConfigs> debugStates = getConfig(GMConfigs::Debug).asDebugConfig();
	debugStates.set(GMDebugConfigs::CalculateBSPFace_Bool, true);
	debugStates.set(GMDebugConfigs::DrawPolygonsAsLine_Bool, false);
	debugStates.set(GMDebugConfigs::DrawSkyOnly_Bool, false);
	debugStates.set(GMDebugConfigs::DrawPolygonNormalMode_I32, GMStates_DebugOptions::DRAW_NORMAL_OFF);
	debugStates.set(GMDebugConfigs::DrawLightmapOnly_Bool, false);
	debugStates.set(GMDebugConfigs::RunProfile_Bool, false);
	debugStates.set(GMDebugConfigs::FrameBufferPositionX_I32, 10);
	debugStates.set(GMDebugConfigs::FrameBufferPositionY_I32, 10);
	debugStates.set(GMDebugConfigs::FrameBufferWidth_I32, 250);
	debugStates.set(GMDebugConfigs::FrameBufferHeight_I32, 250);
	debugStates.set(GMDebugConfigs::FrameBufferIndex_I32, 0);

	GMConfigWrapper<GMRenderConfigs> renderStates = getConfig(GMConfigs::Render).asRenderConfig();
	renderStates.set(GMRenderConfigs::RenderMode_I32, (GMint) GMRenderMode::Forward);
	renderStates.set(GMRenderConfigs::Effects_I32, GMEffects::None);
	renderStates.set(GMRenderConfigs::BLUR_SAMPLE_OFFSET_X, GMStates_RenderOptions::AUTO_SAMPLE_OFFSET);
	renderStates.set(GMRenderConfigs::BLUR_SAMPLE_OFFSET_Y, GMStates_RenderOptions::AUTO_SAMPLE_OFFSET);
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