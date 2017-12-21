#ifndef __GMUI_CONSOLE_H__
#define __GMUI_CONSOLE_H__

#include <gmuicommon.h>
#include <gamemachine.h>
#include "gmuidef.h"
#include "gmuiguiwindow.h"
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
		gm::GMThreadHandle::id id;
		gm::GMint level;
	};

	struct Message
	{
		OutputType type;
		gm::GMString message;
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
	Queue<Message> msgQueue;
	List<Message> msgBuffer;
	gm::Bitset filter;
	gm::GMint tabIndex = 0;

	Map<gm::GMThreadHandle::id, Vector<ProfileInfo> > profiles;
};

class GMUIConsole :
	public GMUIGUIWindow,
	public DuiLib::INotifyUI,
	public gm::IDebugOutput,
	public gm::IProfileHandler
{
	DECLARE_PRIVATE_AND_BASE(GMUIConsole, GMUIGUIWindow)

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
	virtual GMUIStringPtr getWindowClassName() const override;
	virtual gm::GMuint getClassStyle() const override { return CS_DBLCLKS; }
	virtual LongResult onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual LongResult onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual LongResult onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
	virtual void onFinalMessage(gm::GMWindowHandle wndHandle) override;
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
	virtual void beginProfile(const gm::GMString& name, gm::GMfloat durationSinceStartInSecond, gm::GMThreadHandle::id id, gm::GMint level) override;
	virtual void endProfile(const gm::GMString& name, gm::GMfloat elapsedInSecond, gm::GMThreadHandle::id id, gm::GMint level) override;

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