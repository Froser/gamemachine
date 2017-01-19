#ifndef __ALGORITHMN_H__
#define __ALGORITHMN_H__
#include "common.h"
BEGIN_NS

class ChildObject;
class Algorithm
{
public:
	static void createSphere(GMfloat radius, GMint slices, GLint stacks, OUT ChildObject** obj);
};

END_NS
#endif