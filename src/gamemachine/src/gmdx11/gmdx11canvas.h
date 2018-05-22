#ifndef __GMDX11CANVAS_H__
#define __GMDX11CANVAS_H__
#include <gmcommon.h>
#include <gmcanvas.h>
BEGIN_NS

class GMDx11Canvas : public GMCanvas
{
public:
	virtual void onRender() override;
};
END_NS
#endif