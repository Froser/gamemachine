#include "stdafx.h"
#include "gmui_console.h"
#include <gamemachine.h>
#if _WINDOWS
#	include "gmui_console_ui.h"
#endif
#include <gmmessages.h>

enum
{
	TAB_INDEX_LOG = 0,
	TAB_INDEX_PERFORMANCE,
};

GMUIConsole::~GMUIConsole()
{
	if (gm::GMDebugger::getDebugOutput() == this)
		gm::GMDebugger::setDebugOutput(nullptr);
	GM_PROFILE_CLEAR_HANDLER();
}

GMUIStringPtr GMUIConsole::getWindowClassName() const
{
	return _L("gamemachine_Console_class");
}

#if _WINDOWS
template <typename T>
inline static T* findControl(GMUIPainter* painter, gm::GMWchar* name)
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
	GMUIDialogBuilder db(this);
	DuiLib::CControlUI* pRoot = builder.Create(gmui::GMUIConsole::UI, (UINT)0, &db, d->painter);
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

LongResult GMUIConsole::onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	D(d);
	while (!d->msgQueue.empty())
	{
		Data::Message& str = d->msgQueue.front();
		insertTextToRichEdit(str.type, str.message);
		d->msgQueue.pop();
	}
	return 0;
}

void GMUIConsole::onFinalMessage(gm::GMWindowHandle wndHandle)
{
	delete this;
}

void GMUIConsole::Notify(DuiLib::TNotifyUI& msg)
{
	D(d);
	if (msg.sType == DUI_MSGTYPE_CLICK)
	{
		if (msg.pSender == d->optLog)
			selectTab(TAB_INDEX_LOG);
		else if (msg.pSender == d->optPerformance)
			selectTab(TAB_INDEX_PERFORMANCE);
		else
		{
			Data::OutputType type;
			if (msg.pSender == d->optFltInfo)
				type = Data::OutputType::Info;
			else if (msg.pSender == d->optFltWarning)
				type = Data::OutputType::Warning;
			else if (msg.pSender == d->optFltError)
				type = Data::OutputType::Error;
			else if (msg.pSender == d->optFltDebug)
				type = Data::OutputType::Debug;

			d->filter.toggle((gm::GMint) type);
			onFilterChanged();
		}
	}
	else if (msg.sType == DUI_MSGTYPE_WINDOWINIT)
	{
		refreshTabs();
		refreshOptFilter();
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
	d->optFltInfo = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_FILTER_INFO);
	d->optFltWarning = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_FILTER_WARNING);
	d->optFltError = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_FILTER_ERROR);
	d->optFltDebug = findControl<DuiLib::COptionUI>(d->painter, ID_OPTION_FILTER_DEBUG);
	d->profileGraph = findControl<GMUIGraph>(d->painter, ID_PROFILE_GRAPH);
}

void GMUIConsole::selectTab(gm::GMint i)
{
	D(d);
	d->tabIndex = i;
	d->tabLayout->SelectItem(i);
	refreshTabs();
}

void GMUIConsole::addBuffer(Data::OutputType type, const gm::GMString& msg)
{
	D(d);
	const static gm::GMuint MAX_BUFFER = 1000;
	d->msgBuffer.push_back ({ type, msg });
	if (d->msgBuffer.size() > MAX_BUFFER)
		d->msgBuffer.pop_front();
}

void GMUIConsole::onFilterChanged()
{
	D(d);
	refreshOptFilter();

	d->consoleEdit->SetText(_L(""));
	for (auto& msg : d->msgBuffer)
	{
		insertText(msg.type, msg.message);
	}
}

void GMUIConsole::refreshTabs()
{
	D(d);
	gm::GMint i = 0;
	auto tabOptions = { d->optLog, d->optPerformance };
	for (auto opt : tabOptions)
	{
		if (i == d->tabIndex)
		{
			opt->SetBorderSize(1);
			opt->SetBorderColor(0xFFFFFF);
		}
		else
		{
			opt->SetBorderSize(1);
			opt->SetBorderColor(0x000000);
		}
		i++;
	}
}

void GMUIConsole::refreshOptFilter()
{
	D(d);
	auto typeList =  { Data::OutputType::Info,	Data::OutputType::Warning,	Data::OutputType::Error,	Data::OutputType::Debug };
	auto uiList   =  { d->optFltInfo,			d->optFltWarning,			d->optFltError,				d->optFltDebug };
	
	auto iter = uiList.begin();
	for (auto type : typeList)
	{
		DuiLib::COptionUI* opt = const_cast<DuiLib::COptionUI*>(*iter);
		if (d->filter.isSet((gm::GMint) type))
		{
			opt->SetBorderSize(1);
			opt->SetBorderColor(0xFFFFFF);
		}
		else
		{
			opt->SetBorderSize(1);
			opt->SetBorderColor(0x000000);
		}
		iter++;
	}
}

void GMUIConsole::info(const gm::GMString& msg)
{
	D(d);
	addBuffer(Data::OutputType::Info, msg);
	insertText(Data::OutputType::Info, msg);
}

void GMUIConsole::warning(const gm::GMString& msg)
{
	D(d);
	addBuffer(Data::OutputType::Warning, msg);
	insertText(Data::OutputType::Warning, msg);
}

void GMUIConsole::error(const gm::GMString& msg)
{
	D(d);
	addBuffer(Data::OutputType::Error, msg);
	insertText(Data::OutputType::Error, msg);
}

void GMUIConsole::debug(const gm::GMString& msg)
{
	D(d);
	addBuffer(Data::OutputType::Debug, msg);
	insertText(Data::OutputType::Debug, msg);
}

bool GMUIConsole::event(const gm::GameMachineMessage& msg)
{
	D(d);
	centerWindow();
	showWindowEx(true, true);

	if (msg.type == GMM_CONSOLE_SELECT_FILTER)
	{
		d->filter.clearAll();
		d->filter.set(msg.value);
		onFilterChanged();
	}
	return true;
}

void GMUIConsole::insertText(Data::OutputType type, const gm::GMString& msg)
{
	D(d);
	if (isWindowVisible())
	{
		if (d->filter.isSet((gm::GMint)type))
			insertTextToRichEdit(type, msg);
	}
	else
	{
		d->msgQueue.push({ type, msg });
	}
}

void GMUIConsole::insertTextToRichEdit(Data::OutputType type, const gm::GMString& msg)
{
	D(d);
	DWORD color;
	switch (type)
	{
	case Data::OutputType::Info:
		color = 0x555555;
		break;
	case Data::OutputType::Warning:
		color = 0x00FFFF;
		break;
	case Data::OutputType::Error:
		color = 0x3300CC;
		break;
	case Data::OutputType::Debug:
		color = 0xFF0099;
		break;
	default:
		break;
	}

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

void GMUIConsole::beginProfile(const gm::GMString& name, gm::GMfloat durationSinceStartInSecond, gm::GMThreadHandle::id id, gm::GMint level)
{
	D(d);
	gm::GMMutex m;
	if (!isWindowVisible() || d->tabIndex != TAB_INDEX_PERFORMANCE)
		return;

	Data::ProfileInfo info = { name, 0, durationSinceStartInSecond, id, level };
	d->profiles[id].push_back(info);
}

void GMUIConsole::endProfile(const gm::GMString& name, gm::GMfloat elapsedInSecond, gm::GMThreadHandle::id id, gm::GMint level)
{
	D(d);
	gm::GMMutex m;
	if (!isWindowVisible() || d->tabIndex != TAB_INDEX_PERFORMANCE)
		return;

	auto& infos = d->profiles[id];
	auto& targetInfo = std::find_if(infos.begin(), infos.end(), [id, level, &name](Data::ProfileInfo& info) {
		return info.name == name && info.id == id && info.level == level;
	});
	ASSERT(targetInfo != infos.end());
	targetInfo->durationInSecond = elapsedInSecond;
}

void GMUIConsole::update()
{
	// 在这里更新绘制数据
	static constexpr gm::GMlong colors[] = {
		0x0033FF,
		0x00CC00,
		0x00CCFF,
		0x6600CC,
	};
	static constexpr decltype(sizeof(colors)) colorLen = sizeof(colors) / sizeof(colors[0]);
	static constexpr gm::GMint magnification = 5000;
	GM_PROFILE_RESET_TIMELINE();

	D(d);
	if (!isWindowVisible() || d->tabIndex != TAB_INDEX_PERFORMANCE)
		return;

	GMUIGraphGuard guard(d->profileGraph);
	d->profileGraph->clearGraph();

	if (!GMGetDebugState(RUN_PROFILE))
	{
		d->profileGraph->drawText(_L("Profile function disabled. "));
		return;
	}

	for (auto& profile : d->profiles)
	{
		d->profileGraph->penReturn(12);
		d->profileGraph->penEnter();

		for (auto& info : profile.second)
		{
			gm::GMint w = (gm::GMint) info.durationInSecond * magnification, h = 12;
			gm::GMint start = (gm::GMint) info.durationSinceStartInSecond * magnification;

			d->profileGraph->penForward(start, 0);
			d->profileGraph->drawText(info.name);
			d->profileGraph->penReturn(12);
			d->profileGraph->penEnter();
			d->profileGraph->penForward(start, 0);

			const gm::GMlong& color = colors[info.level % colorLen];
			d->profileGraph->drawRect(color, w, h);
			d->profileGraph->penForward(12, 0);
			d->profileGraph->drawText(gm::GMString(info.durationInSecond * 1000) + _L("ms"));
			d->profileGraph->penReturn(24);
			d->profileGraph->penEnter();
		}
		profile.second.clear();
	}
	d->profileGraph->endDraw();
}

#endif
