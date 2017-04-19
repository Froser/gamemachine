#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "common.h"
#include <map>
BEGIN_NS

// Debug options:
enum DebugOptions
{
	CALCULATE_BSP_FACE,
	POLYGON_LINE_MODE,
	DRAW_ONLY_SKY,
	DRAW_NORMAL, // see DrawNormalOptions

	DEBUG_OPTIONS_END,
};

//Draw normal options
enum DrawNormalOptions
{
	DRAW_NORMAL_OFF,
	DRAW_NORMAL_EYESPACE,
	DRAW_NORMAL_CAMERASPACE,
	DRAW_NORMAL_MAX
};

#define DBG Debug::getDebugger()
#define DBG_SET_INT(key, value) DBG->setInt(key, value)
#define DBG_INT(key) DBG->getInt(key)

class Debug
{
public:
	static Debug* getDebugger();

public:
	void setInt(GMint key, GMint value);
	GMint getInt(GMint key);

private:
	Debug();

private:
	std::map<GMint, GMint> m_ints;
};

inline void format_time(char* in)
{
#ifdef _WINDOWS
	SYSTEMTIME time = { 0 };
	GetLocalTime(&time);
	sprintf_s(in, LINE_MAX, "%d-%02d-%02d %02d:%02d:%02d",
		time.wYear,
		time.wMonth,
		time.wDay,
		time.wHour,
		time.wMinute,
		time.wSecond
	);
#endif
}

#define f_time(t) char t[LINE_MAX]; format_time(t);
inline void gm_print(const char *format, ...)
{
	f_time(t);
	printf("%s: ", t);
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("\n");
}

// debug macros:
#define gm_info gm_print
#define gm_error gm_print
#define gm_warning gm_print

END_NS
#endif