#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "common.h"
BEGIN_NS

class Image;
struct ITexture;
class ObjectPainter;
struct IGraphicEngine;
class Object;
struct IFactory
{
	virtual ~IFactory();
	virtual void createGraphicEngine(OUT IGraphicEngine**) = 0;
	virtual void createTexture(Image*, OUT ITexture**) = 0;
	virtual void createPainter(IGraphicEngine*, Object*, OUT ObjectPainter**) = 0;
};

END_NS
#endif