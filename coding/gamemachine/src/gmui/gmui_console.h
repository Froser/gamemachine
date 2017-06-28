#ifndef __GMUI_CONSOLE_H__
#define __GMUI_CONSOLE_H__
#include "common.h"
#include "gmui.h"
#include <queue>
#include <list>
#include "foundation/utilities/utilities.h"
#include "foundation/gmprofile.h"
#include "gmuicontrols.h"
BEGIN_NS

#if _WINDOWS
GM_PRIVATE_OBJECT(GMUIConsole)
{
	enum class OutputType
	{
		Info = 0,
		Warning,
		Error,
		Debug,
		EndOfOutputType, //结束标记
	};

	struct ProfileInfo
	{
		GMString name;
		GMfloat durationInSecond;
		GMfloat durationSinceStartInSecond;
		GMint id;
		GMint level;
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
	IUIGraph* profileGraph;
	GMUIPainter* painter;
	std::queue<Message> msgQueue;
	std::list<Message> msgBuffer;
	Bitset filter;
	GMint tabIndex = 0;

	std::map<GMint, Vector<ProfileInfo> > profiles;
};

class GMUIConsole :
	public GMUIGUIWindow,
	public DuiLib::INotifyUI,
	public IDebugOutput,
	public IProfileHandler
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
		GM_PROFILE_HANDLER(this);
		d->painter = &db->painter;
		d->filter.init( (GMint) Data::OutputType::EndOfOutputType);
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
	virtual void update();

public:
	virtual void Notify(DuiLib::TNotifyUI& msg) override;

	// IDebugOutput
public:
	virtual void info(const GMString& msg) override;
	virtual void warning(const GMString& msg) override;
	virtual void error(const GMString& msg) override;
	virtual void debug(const GMString& msg) override;

	// IProfileHandler
public:
	virtual void begin(GMint id, GMint level) override;
	virtual void output(const GMString& name, GMfloat timeInSecond, GMfloat durationSinceLast, GMint id, GMint level) override;
	virtual void end(GMint id, GMint level) override;

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