#ifndef __GMUI_CONSOLE_H__
#define __GMUI_CONSOLE_H__
#include "common.h"
#include "gmui.h"
#include <queue>
#include <list>
#include "foundation/utilities/utilities.h"
BEGIN_NS

#if _WINDOWS
GM_PRIVATE_OBJECT(GMUIConsole)
{
	enum OutputType
	{
		Info = 0,
		Warning,
		Error,
		Debug,
		EndOfOutputType, //结束标记
	};

	struct Message
	{
		OutputType type;
		GMString message;
	};

	DuiLib::CRichEditUI* consoleEdit;
	DuiLib::CTabLayoutUI* tabLayout;
	DuiLib::COptionUI* optLog;
	DuiLib::COptionUI* optPerformance;
	DuiLib::COptionUI* optFltInfo;
	DuiLib::COptionUI* optFltWarning;
	DuiLib::COptionUI* optFltError;
	DuiLib::COptionUI* optFltDebug;
	GMUIPainter* painter;
	std::queue<Message> msgQueue;
	std::list<Message> msgBuffer;
	Bitset filter;
	GMint tabIndex = 0;
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
		d->filter.init(Data::EndOfOutputType);
		d->filter.setAll();
	}

	~GMUIConsole();

public:
	virtual GMUIStringPtr getWindowClassName() const override;
	virtual GMuint getClassStyle() const override { return CS_DBLCLKS; }
	virtual LongResult onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual LongResult onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual LongResult onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual void onFinalMessage(GMUIWindowHandle wndHandle) override;

public:
	virtual void Notify(DuiLib::TNotifyUI& msg) override;

public:
	virtual void info(const GMString& msg) override;
	virtual void warning(const GMString& msg) override;
	virtual void error(const GMString& msg) override;
	virtual void debug(const GMString& msg) override;

private:
	void insertText(Data::OutputType type, const GMString& msg);
	void insertTextToRichEdit(Data::OutputType type, const GMString& msg);
	void afterCreated();
	void selectTab(GMint i);
	void addBuffer(Data::OutputType type, const GMString& msg);
	void onFilterChanged();
	void refreshOptFilter();
	void refreshTabs();
};

#endif

END_NS
#endif