#ifndef __OBJECT_DRAWER_H__
#define __OBJECT_DRAWER_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "gmglshaders.h"
BEGIN_NS

class GMGLObjectDrawer : public ObjectDrawer
{
public:
	virtual void init(Object*) override;
	virtual void draw(GMGLShaders&, Object*) override;
	virtual void dispose(Object*) override;
};

END_NS
#endif