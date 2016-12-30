#ifndef __ALGORITHMN_H__
#define __ALGORITHMN_H__
#include "common.h"
BEGIN_NS

class Object;
class Algorithm
{
public:
	static void createSphere(GMfloat radius, GMint slices, GLint stacks, OUT Object** obj);
};

END_NS
#endif