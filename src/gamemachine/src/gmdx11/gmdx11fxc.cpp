#include "stdafx.h"
#include "gmdx11fxc.h"
#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <strsafe.h>

#define TEMP_CODE_FILE "code.hlsl"
#define BUFSIZE 10 * 1024

namespace
{
	void ShowPipeContentsAndClose(HANDLE hPipe)
	{
		DWORD dwRead = 0;
		CHAR chBuf[BUFSIZE] = { 0 };
		BOOL bSuccess = FALSE;

		for (;;)
		{
			bSuccess = ReadFile(hPipe, chBuf, BUFSIZE, NULL, NULL);
			if (!bSuccess) break;
		}

		gm_info(gm_dbg_wrap("%s"), chBuf);
		CloseHandle(hPipe);
	}
}

void GMDx11FXC::init()
{
	do 
	{
		if (!findFXC())
			break;

		m_available = true;
	} while (false);
}

bool GMDx11FXC::compile(const GMDx11FXCDescription& d)
{
	if (!isAvailable())
		return false;

	GMDx11FXCDescription desc;
	desc = d;

	if (!fillDescription(&desc))
		return false;
	// 首先，将代码文件写入desc.tempDir
	WCHAR szTemp[MAX_PATH];
	PathCombine(szTemp, desc.tempDir.toStdWString().c_str(), _T(TEMP_CODE_FILE));
	HANDLE hFile = CreateFile(szTemp, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile)
		return false;

	std::string code = desc.code.toStdString();
	WriteFile(hFile, code.c_str(), (DWORD) code.size(), NULL, NULL);
	CloseHandle(hFile);

	// 获取fxc.exe目录以及路径
	PROCESS_INFORMATION piInfo;
	std::wstring fxcPath = m_fxc.toStdWString();
	_tcscpy_s(szTemp, MAX_PATH, fxcPath.c_str());
	PathRemoveFileSpec(szTemp);

	// 创建管道重定向输出
	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;
	HANDLE hStdRead = NULL;
	HANDLE hStdWrite = NULL;
	CreatePipe(&hStdRead, &hStdWrite, &saAttr, 0);

	STARTUPINFO siInfo = { 0 };
	siInfo.cb = sizeof(STARTUPINFO);
	siInfo.wShowWindow = SW_HIDE;
	siInfo.hStdOutput = hStdWrite;
	siInfo.hStdError = hStdWrite;
	siInfo.hStdInput = hStdRead;
	siInfo.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	BOOL bResult = CreateProcess(
		NULL,
		(LPWSTR) fxcPath.c_str(),
		NULL,
		NULL,
		FALSE,
		NULL,
		NULL,
		szTemp,
		&siInfo,
		&piInfo
	);

	if (!bResult)
		return false;

	WaitForSingleObject(piInfo.hProcess, INFINITE);
	ShowPipeContentsAndClose(hStdWrite);
	CloseHandle(piInfo.hProcess);
	CloseHandle(piInfo.hThread);
	return true;
}

bool GMDx11FXC::findFXC()
{
	bool available = false;
	WCHAR szSDKPath[MAX_PATH];
	DWORD dwResult = GetEnvironmentVariable(_T("DXSDK_DIR"), szSDKPath, MAX_PATH);
	if (dwResult == 0)
	{
		available = false;
	}
	else
	{
		PathAddBackslash(szSDKPath);
		LPTSTR szFXCSuffix = _T("Utilities\\bin\\x86\\fxc.exe");
		_tcscat_s(szSDKPath, MAX_PATH, szFXCSuffix);
		if (PathFileExists(szSDKPath))
		{
			available = true;
			m_fxc = GMString(szSDKPath);
		}
	}
	return available;
}

bool GMDx11FXC::fillDescription(GMDx11FXCDescription* desc)
{
	GM_ASSERT(desc);
	WCHAR szTempPath[MAX_PATH];
	if (desc->tempDir.isEmpty())
	{
		GetModuleFileNameW(NULL, szTempPath, MAX_PATH);
		PathRemoveFileSpec(szTempPath);
		PathCombine(szTempPath, szTempPath, _T("temp"));
		desc->tempDir = GMString(szTempPath);
		if (!PathFileExists(szTempPath) && !CreateDirectory(szTempPath, NULL))
			return false;
	}

	if (desc->fxc.isEmpty())
	{
		GetModuleFileNameW(NULL, szTempPath, MAX_PATH);
		PathRemoveFileSpec(szTempPath);
		PathCombine(szTempPath, szTempPath, _T("prefetch"));
		desc->fxc = GMString(szTempPath);
		if (!PathFileExists(szTempPath) && !CreateDirectory(szTempPath, NULL))
			return false;
	}

	return true;
}

