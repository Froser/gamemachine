#include "stdafx.h"
#include "check.h"
#include "../../utilities/tools.h"

GMString GMConvertion::toCurrentEnvironmentString(const GMString& string)
{
	return toUnixString(string);
}