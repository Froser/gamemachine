#include "stdafx.h"
#include "gmui_console.h"
#if _WINDOWS
#	include "gmui_console_ui.h"
#endif

GMUIConsole::~GMUIConsole()
{
	if (GMDebugger::getDebugOutput() == this)
		GMDebugger::setDebugOutput(nullptr);
}

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
template <typename T>
inline static T* findControl(GMUIPainter* painter, GMWchar* name)
{
	T* control = static_cast<T*>(painter->FindControl(name));
	ASSERT(control);
	return control;
}

LongResult GMUIConsole::onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	D(d);
	d->painter->Init(getWindowHandle());
	DuiLib::CDialogBuilder builder;
	//CDialogBuilderCallbackEx cb;
	DuiLib::CControlUI* pRoot = builder.Create(gmui::GMUIConsole::UI, (UINT)0, NULL, d->painter);
	ASSERT(pRoot && "Failed to parse XML");
	d->painter->AttachDialog(pRoot);
	d->painter->AddNotifier(this);
	LongResult result = Base::onCreate(uMsg, wParam, lParam, bHandled);

	afterCreated();
	bHandled = TRUE;
	return result;
}

LongResult GMUIConsole::onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	hideWindow();
	bHandled = TRUE;
	return 0;
}

void GMUIConsole::onFinalMessage(GMUIWindowHandle wndHandle)
{
	delete this;
}

void GMUIConsole::Notify(DuiLib::TNotifyUI& msg)
{
	D(d);
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender == d->optLog)
			d->tabLayout->SelectItem(0);
		else if (msg.pSender == d->optPerformance)
			d->tabLayout->SelectItem(1);
	}
}

void GMUIConsole::afterCreated()
{
	D(d);
	DuiLib::CRichEditUI* re = static_cast<DuiLib::CRichEditUI*> (d->painter->FindControl(ID_EDIT_CONSOLE));
	ASSERT(re);
	d->consoleEdit = re;
	d->consoleEdit->SetBkColor(0);

	d->tabLayout = findControl<DuiLib::CTabLayoutUI>(d->painter, ID_TABLAYOUT);
	d->optLog = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_LOG);
	d->optPerformance = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_PERFORMACE);
}

void GMUIConsole::info(const GMString& msg)
{
	D(d);
	insertText(Data::Info, msg, 0x555555);
}

void GMUIConsole::warning(const GMString& msg)
{
	D(d);
	insertText(Data::Warning, msg, 0x00FFFF);
}

void GMUIConsole::error(const GMString& msg)
{
	D(d);
	insertText(Data::Error, msg, 0x3300CC);
}

void GMUIConsole::debug(const GMString& msg)
{
	D(d);
	insertText(Data::Debug, msg, 0xFF0099);
}

void GMUIConsole::insertText(Data::OutputType type, const GMString& msg, DWORD color)
{
	D(d);
	if (isWindowVisible())
	{
		CHARRANGE cr1, cr2;
		d->consoleEdit->GetSel(cr1);
		d->consoleEdit->InsertText(0, (msg + _L("\n")).toStdWString().c_str());
		d->consoleEdit->EndRight();
		d->consoleEdit->GetSel(cr2);
		d->consoleEdit->SetSel(cr1.cpMin, cr2.cpMax);
		CHARFORMAT2 cf;
		d->consoleEdit->GetSelectionCharFormat(cf);
		cf.crTextColor = color;
		d->consoleEdit->SetSelectionCharFormat(cf);
	}
	else
	{
		d->msgQueue.push({ type, msg });
	}
}

#endif
