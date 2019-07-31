#include "stdafx.h"
#include "check.h"
#include "defines.h"
#include "foundation/gamemachine.h"
#include "foundation/gamemachine_p.h"
BEGIN_NS

namespace
{
	GMEndiannessMode getMachineEndianness()
	{
		long int i = 1;
		const char *p = (const char *)&i;
		if (p[0] == 1)
			return GMEndiannessMode::LittleEndian;
		return GMEndiannessMode::BigEndian;
	}
}

void GameMachinePrivate::runEventLoop()
{
	P_D(pd);
	MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	BOOL bGotMessage = FALSE;
	while (WM_QUIT != msg.message)
	{
		bGotMessage = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
		if (bGotMessage)
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			if (runningMode == GMGameMachineRunningMode::GameMode)
			{
				if (!pd->renderFrame())
					break;
			}
		}

		// Application模式下，虽然不进行渲染，但是还是有消息处理
		if (!pd->handleMessages())
			break;
	}
	pd->finalize();
}

void GameMachinePrivate::initSystemInfo()
{
	static bool inited = false;
	if (!inited)
	{
		states.systemInfo.endiannessMode = getMachineEndianness();
		
		SYSTEM_INFO sysInfo;
		::GetSystemInfo(&sysInfo);
		states.systemInfo.numberOfProcessors = sysInfo.dwNumberOfProcessors;

		switch (sysInfo.wProcessorArchitecture)
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			states.systemInfo.processorArchitecture = GMProcessorArchitecture::AMD64;
			break;
		case PROCESSOR_ARCHITECTURE_ARM:
			states.systemInfo.processorArchitecture = GMProcessorArchitecture::ARM;
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			states.systemInfo.processorArchitecture = GMProcessorArchitecture::IA64;
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			states.systemInfo.processorArchitecture = GMProcessorArchitecture::Intel;
			break;
		case PROCESSOR_ARCHITECTURE_UNKNOWN:
		default:
			states.systemInfo.processorArchitecture = GMProcessorArchitecture::Unknown;
			break;
		}
		inited = true;
	}
}

END_NS