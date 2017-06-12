#include "stdafx.h"
#include "gmui_console.h"
#if _WINDOWS
#include "gmui_console_ui.h"
#endif

void GMUIConsole::newConsoleWindow(OUT GMUIConsole** out)
{
	ASSERT(*out);
	*out = new GMUIConsole();
}

GMUIStringPtr GMUIConsole::getWindowClassName() const
{
	return _L("gamemachine_Console_class");
}

#if _WINDOWS
LongResult GMUIConsole::onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	D(d);
	d->painter.Init(getWindowHandle());
	DuiLib::CDialogBuilder builder;
	//CDialogBuilderCallbackEx cb;
	DuiLib::CControlUI* pRoot = builder.Create(gmui::GMUIConsole::UI, (UINT)0, NULL, &d->painter);
	ASSERT(pRoot && "Failed to parse XML");
	d->painter.AttachDialog(pRoot);
	d->painter.AddNotifier(this);
	return Base::onCreate(uMsg, wParam, lParam, bHandled);
}

void GMUIConsole::onFinalMessage(GMUIWindowHandle wndHandle)
{
	delete this;
}


void GMUIConsole::Notify(DuiLib::TNotifyUI& msg)
{
	if (msg.sType == _T("windowinit")) 
		onPrepare();
}

void GMUIConsole::onPrepare()
{
}

void GMUIConsole::info(const GMString& msg)
{
	richEdit()->AppendText((msg + _L("\n")).toStdWString().c_str());
}

void GMUIConsole::warning(const GMString& msg)
{
}

void GMUIConsole::error(const GMString& msg)
{
}

void GMUIConsole::debug(const GMString& msg)
{
}

inline DuiLib::CRichEditUI* GMUIConsole::richEdit()
{
	D(d);
	DuiLib::CRichEditUI* re = static_cast<DuiLib::CRichEditUI*> (d->painter.FindControl(_L("Edit_Console")));
	ASSERT(re);
	return re;
}

#endif
