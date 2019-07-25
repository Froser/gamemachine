#ifndef __GMCONTROLS_P_H__
#define __GMCONTROLS_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMControl)
{
	GMint32 x = 0;
	GMint32 y = 0;
	GMint32 width = 0;
	GMint32 height = 0;
	GMRect boundingBox;
	GMWidget* widget = nullptr;

	bool styleInited = false;
	bool enabled = true;
	bool visible = true;
	bool mouseOver = false;
	bool hasFocus = false;
	bool isDefault = false;

	GMint32 index = 0; //在控件列表中的索引
	GMControlPositionFlag positionFlag = GMControlPositionFlag::Auto;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMControlLabel)
{
	GMString text;
	GMStyle foreStyle;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMControlButton)
{
	bool pressed = false;
	GMStyle fillStyle;
	GMOwnedPtr<GMControlBorder> fillBorder;
	GMOwnedPtr<GMControlBorder> foreBorder;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMControlBorder)
{
	GMStyle borderStyle;
	GMRect corner;
};

END_NS
#endif