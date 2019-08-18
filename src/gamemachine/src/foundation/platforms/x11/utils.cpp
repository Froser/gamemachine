#include "stdafx.h"
#include "check.h"
#include "../../utilities/tools.h"

BEGIN_NS

GMString GMConvertion::toCurrentEnvironmentString(const GMString& string)
{
	return toUnixString(string);
}

END_NS