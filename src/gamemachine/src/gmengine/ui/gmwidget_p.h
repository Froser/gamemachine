#ifndef __GMWIDGET_P_H__
#define __GMWIDGET_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMWidget)
{
	GMWidgetResourceManager* manager = nullptr;
	IWindow* parentWindow = nullptr;
	GMWidget* nextWidget; // 下一个Widget默认为自己
	GMWidget* prevWidget; // 上一个Widget默认为自己
	GM_OWNED Vector<GMControl*> controls;
	GMControl* focusControl = nullptr;
	GMControlBorder* borderControl = nullptr;
	GMint32 borderMarginLeft = 10;
	GMint32 borderMarginTop = 30;
	GMint32 contentPaddingLeft = 0;
	GMint32 contentPaddingTop = 0;
	GMint32 contentPaddingRight = 0;
	GMint32 contentPaddingBottom = 0;
	GMfloat timeLastRefresh = 0;
	GMControl* controlMouseOver = nullptr;
	bool nonUserEvents = false;
	bool keyboardInput = false;
	bool mouseInput = false;
	bool visible = true;
	bool minimized = false;
	bool title = false;
	GMint32 titleHeight = 20;
	GMString titleText;
	GMPoint titleOffset;
	GMStyle titleStyle;
	GMStyle shadowStyle;

	GMStyle whiteTextureStyle;
	GMOverflowStyle overflow = GMOverflowStyle::Auto;

	GMFloat4 colorTopLeft = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorTopRight = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorBottomLeft = GMFloat4(0, 0, 0, 0);
	GMFloat4 colorBottomRight = GMFloat4(0, 0, 0, 0);

	GMint32 width = 0;
	GMint32 height = 0;
	GMint32 x = 0;
	GMint32 y = 0;
	HashMap<GMTextureArea::Area, GMWidgetTextureArea> areas;

	bool movingWidget = false;
	GMPoint movingStartPt;

	GMint32 scrollOffsetY = 0;
	GMint32 scrollStep = 10;
	GMRect controlBoundingBox = { 0 };
	GMOwnedPtr<GMControlScrollBar> verticalScrollbar;
	GMint32 verticalScrollbarWidth;

	GMfloat scaling[2] = { 0 };
};

END_NS
#endif