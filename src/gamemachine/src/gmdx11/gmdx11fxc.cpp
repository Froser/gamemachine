#include "stdafx.h"
#include "gmdx11fxc.h"
#include <windows.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <strsafe.h>

#define TEMP_CODE_FILE "code.hlsl"
#define LOG_FILE "log.txt"
#define BUFSIZE 10 * 1024

namespace
{
	void ShowContentsAndClose(HANDLE hFile)
	{
		DWORD dwRead = 0;
		CHAR chBuf[BUFSIZE] = { 0 };
		ReadFile(hFile, chBuf, BUFSIZE, &dwRead, NULL);
		gm_info(gm_dbg_wrap("%s"), chBuf);
		CloseHandle(hFile);
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
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	std::string code = desc.code.toStdString();
	WriteFile(hFile, code.c_str(), (DWORD) code.size(), NULL, NULL);
	CloseHandle(hFile);

	// 创建文件重定向输出
	SECURITY_ATTRIBUTES saAttr = { sizeof(SECURITY_ATTRIBUTES) };
	saAttr.bInheritHandle = FALSE;
	saAttr.lpSecurityDescriptor = NULL;
	PathCombine(szTemp, desc.tempDir.toStdWString().c_str(), _T(LOG_FILE));
	HANDLE hStdWrite = CreateFile(
		szTemp,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		&saAttr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hStdWrite == INVALID_HANDLE_VALUE)
		return false;

	// 获取fxc.exe目录以及路径
	PROCESS_INFORMATION piInfo;
	std::wstring fxcPath = m_fxc.toStdWString();
	_tcscpy_s(szTemp, MAX_PATH, fxcPath.c_str());
	PathRemoveFileSpec(szTemp);

	STARTUPINFO siInfo = { 0 };
	siInfo.cb = sizeof(STARTUPINFO);
	siInfo.hStdOutput = hStdWrite;
	siInfo.hStdError = hStdWrite;
	siInfo.hStdInput = NULL;
	siInfo.dwFlags |= STARTF_USESTDHANDLES;

	TCHAR szFXCPath[MAX_PATH] = { 0 };
	GMString::stringCopy(szFXCPath, ("\"" + fxcPath + "\" /?").c_str());
	BOOL bResult = CreateProcess(
		NULL,
		szFXCPath,
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW,
		NULL,
		szTemp,
		&siInfo,
		&piInfo
	);

	if (!bResult)
	{
		CloseHandle(hStdWrite);
		return false;
	}

	WaitForSingleObject(piInfo.hProcess, INFINITE);
	ShowContentsAndClose(hStdWrite);
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

