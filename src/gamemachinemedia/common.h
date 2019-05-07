#ifndef __GMM_COMMON_H__
#define __GMM_COMMON_H__

#include <defines.h>
#include <gmbuffer.h>
#if GM_WINDOWS
#	include <windows.h>
#endif

#define BEGIN_MEDIA_NS namespace gmm {
#define END_MEDIA_NS }

// 导入导出
#pragma warning( disable: 4251 )
#pragma warning( disable: 4275 )

#ifdef GM_MEDIA_DLL
#	ifndef GM_MEDIA_EXPORT
#		define GM_MEDIA_EXPORT GM_DECL_EXPORT
#	endif
#else
#	if GM_USE_MEDIA_DLL
#		ifndef GM_MEDIA_EXPORT
#			define GM_MEDIA_EXPORT GM_DECL_IMPORT
#		endif
#	else
#		ifndef GM_MEDIA_EXPORT
#			define GM_MEDIA_EXPORT
#		endif
#	endif
#endif


// 流采样数目
#define STREAM_BUFFER_NUM 4

#define GMM_SLEEP_FOR_ONE_FRAME() gm::GMThread::sleep(1000 / 60);

// 常用函数
inline bool strnEqual(const char* a, const char* b, int n)
{
	int i = 0;
	do
	{
		if (a[i] != b[i])
			return false;
	} while (++i < n);
	return true;
}

#endif