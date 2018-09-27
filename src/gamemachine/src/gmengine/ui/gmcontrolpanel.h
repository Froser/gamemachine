#ifndef __GMPANEL_H__
#define __GMPANEL_H__
#include <gmcommon.h>
#include <gmcontrols.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMPanel)
{

};

//! 一个面板控件。
/*!
  面板控件可以容纳其他控件。同时它可以防止其他控件绘制区域超出面板的范围。
*/
class GMPanel : public GMControl
{
	GM_DECLARE_PRIVATE(GMPanel)

public:
	virtual void render(float elapsed) override;

};

END_NS
#endif