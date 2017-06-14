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
static void insertText(DuiLib::CRichEditUI* re, const GMString& msg, DWORD color)
{
	CHARRANGE cr1, cr2;
	re->GetSel(cr1);
	re->InsertText(0, (msg + _L("\n")).toStdWString().c_str());
	re->EndRight();
	re->GetSel(cr2);
	re->SetSel(cr1.cpMin, cr2.cpMax);
	CHARFORMAT2 cf;
	re->GetSelectionCharFormat(cf);
	cf.crTextColor = color;
	re->SetSelectionCharFormat(cf);
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
}

void GMUIConsole::afterCreated()
{
	D(d);
	DuiLib::CRichEditUI* re = static_cast<DuiLib::CRichEditUI*> (d->painter->FindControl(_L("Edit_Console")));
	ASSERT(re);
	d->richEdit = re;
	d->richEdit->SetBkColor(0);
}

void GMUIConsole::info(const GMString& msg)
{
	D(d);
	insertText(d->richEdit, msg, 0x555555);
}

void GMUIConsole::warning(const GMString& msg)
{
	D(d);
	insertText(d->richEdit, msg, 0x00FFFF);
}

void GMUIConsole::error(const GMString& msg)
{
	D(d);
	insertText(d->richEdit, msg, 0x3300CC);
}

void GMUIConsole::debug(const GMString& msg)
{
	D(d);
	insertText(d->richEdit, msg, 0xFF0099);
}

#endif
