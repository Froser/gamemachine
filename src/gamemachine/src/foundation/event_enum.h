#ifndef __EVENT_ENUM
#define __EVENT_ENUM
BEGIN_NS

#define GM_CONTROL_EVENT_ENUM(name) GMEVENT_UI_##name

enum __UIEvents
{
	GM_CONTROL_EVENT_ENUM(MouseHover),
	GM_CONTROL_EVENT_ENUM(MouseLeave),
	GM_CONTROL_EVENT_ENUM(MouseDown),
};

END_NS
#endif