#ifndef __GMM_COMMON_H__
#define __GMM_COMMON_H__

#if _WINDOWS
#	include <windows.h>
#endif

#define BEGIN_MEDIA_NS namespace gmm {
#define END_MEDIA_NS }

// 流采样数目
#define STREAM_BUFFER_NUM 4

#define GMM_SLEEP_FOR_ONE_FRAME() gm::GMThread::sleep(1000 / 60);

// 常用函数
inline bool strnEqual(char* a, char* b, int n)
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