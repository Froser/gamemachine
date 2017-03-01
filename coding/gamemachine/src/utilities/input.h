#ifndef __INPUT_H__
#define __INPUT_H__
#include "common.h"

BEGIN_NS
#ifdef _WINDOWS
class Input_Windows
{
public:
	Input_Windows();
	~Input_Windows();

private:
	bool init();
	HRESULT setup();
};
#endif

#ifdef _WINDOWS
typedef Input_Windows Input;
#endif

END_NS
#endif