#ifndef __GMUI_CONSOLE_H__
#define __GMUI_CONSOLE_H__

#include <gmuicommon.h>
#include <gamemachine.h>
#include "../gmuidef.h"
#include "gmuiduilibwindow.h"
#include "gmuicontrols.h"

BEGIN_UI_NS

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
		gm::GMString name;
		gm::GMfloat durationInSecond;
		gm::GMfloat durationSinceStartInSecond;
		gm::GMThreadId id;
		gm::GMint level;
	};

	struct Message
	{
		OutputType type;
		gm::GMString message;
	};

	DuiLib::CRichEditUI* consoleEdit = nullptr;
	DuiLib::CTabLayoutUI* tabLayout = nullptr;
	DuiLib::COptionUI* optLog = nullptr;
	DuiLib::COptionUI* optPerformance = nullptr;
	DuiLib::COptionUI* optFltInfo = nullptr;
	DuiLib::COptionUI* optFltWarning = nullptr;
	DuiLib::COptionUI* optFltError = nullptr;
	DuiLib::COptionUI* optFltDebug = nullptr;
	IUIGraph* profileGraph = nullptr;
	DuiLib::CPaintManagerUI* painter = nullptr;
	Queue<Message> msgQueue;
	List<Message> msgBuffer;
	gm::Bitset filter;
	gm::GMint tabIndex = 0;

	Map<gm::GMThreadId, Vector<ProfileInfo> > profiles;
};

class GMUIConsole :
	public GMUIDuiLibWindow,
	public DuiLib::INotifyUI,
	public gm::IDebugOutput,
	public gm::IProfileHandler
{
	DECLARE_PRIVATE_AND_BASE(GMUIConsole, GMUIDuiLibWindow)

public:
	GMUIConsole()
	{
		D(d);
		D_BASE(db, Base);
		GM_PROFILE_HANDLER(this);
		d->painter = &db->painter;
		d->filter.init( (gm::GMint) Data::OutputType::EndOfOutputType);
		d->filter.setAll();
	}

	~GMUIConsole();

public:
	virtual LPCTSTR getWindowClassName() const override;
	virtual UINT getClassStyle() const override { return CS_DBLCLKS; }
	virtual void onFinalMessage(HWND wndHandle) override;

	virtual bool onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) override;
	virtual bool onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) override;
	virtual bool onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) override;
	virtual void update();

public:
	virtual void Notify(DuiLib::TNotifyUI& msg) override;

	// IDebugOutput
public:
	virtual void info(const gm::GMString& msg) override;
	virtual void warning(const gm::GMString& msg) override;
	virtual void error(const gm::GMString& msg) override;
	virtual void debug(const gm::GMString& msg) override;

	// IProfileHandler
public:
	virtual void beginProfile(const gm::GMString& name, gm::GMfloat durationSinceStartInSecond, gm::GMThreadId id, gm::GMint level) override;
	virtual void endProfile(const gm::GMString& name, gm::GMfloat elapsedInSecond, gm::GMThreadId id, gm::GMint level) override;

	// GMObject
public:
	virtual bool event(const gm::GameMachineMessage& msg) override;

private:
	void insertText(Data::OutputType type, const gm::GMString& msg);
	void insertTextToRichEdit(Data::OutputType type, const gm::GMString& msg);
	void afterCreated();
	void selectTab(gm::GMint i);
	void addBuffer(Data::OutputType type, const gm::GMString& msg);
	void onFilterChanged();
	void refreshOptFilter();
	void refreshTabs();
};

END_UI_NS
#endif