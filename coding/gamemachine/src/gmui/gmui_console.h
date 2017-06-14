#ifndef __GMUI_CONSOLE_H__
#define __GMUI_CONSOLE_H__
#include "common.h"
#include "gmui.h"
BEGIN_NS

#if _WINDOWS
GM_PRIVATE_OBJECT(GMUIConsole)
{
	DuiLib::CRichEditUI* richEdit;
	GMUIPainter* painter;
};

class GMUIConsole : public GMUIGUIWindow, public DuiLib::INotifyUI, public IDebugOutput
{
	DECLARE_PRIVATE(GMUIConsole)

	typedef GMUIGUIWindow Base;

public:
	static void newConsoleWindow(AUTORELEASE OUT GMUIConsole** out);

private:
	GMUIConsole()
	{
		D(d);
		D_BASE(db, Base);
		d->painter = &db->painter;
	}

	~GMUIConsole();

public:
	virtual GMUIStringPtr getWindowClassName() const override;
	virtual GMuint getClassStyle() const override { return CS_DBLCLKS; }
	virtual LongResult onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual LongResult onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual void onFinalMessage(GMUIWindowHandle wndHandle) override;

public:
	virtual void Notify(DuiLib::TNotifyUI& msg) override;

public:
	virtual void info(const GMString& msg) override;
	virtual void warning(const GMString& msg) override;
	virtual void error(const GMString& msg) override;
	virtual void debug(const GMString& msg) override;

private:
	void afterCreated();
};

#endif

END_NS
#endif