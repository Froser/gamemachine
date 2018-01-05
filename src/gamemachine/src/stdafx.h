#if GM_WINDOWS
#include <windows.h>
#endif

#if GM_GCC
#	ifdef __USE_MINGW_ANSI_STDIO
#	undef __USE_MINGW_ANSI_STDIO
#	define __USE_MINGW_ANSI_STDIO 0
#	endif
#endif

namespace gm {};
using namespace gm;