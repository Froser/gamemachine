#ifndef __FACTORY_H__
#define __FACTORY_H__
#include "common.h"
BEGIN_NS

enum GamePackageType
{
	GPT_DIRECTORY,
	GPT_ZIP,
};

struct ITexture;
struct IGraphicEngine;
struct IGamePackageHandler;
class Image;
class Object;
class ObjectPainter;
class GamePackage;
struct IFactory
{
	virtual ~IFactory();
	virtual void createGraphicEngine(OUT IGraphicEngine**) = 0;
	virtual void createTexture(Image*, OUT ITexture**) = 0;
	virtual void createPainter(IGraphicEngine*, Object*, OUT ObjectPainter**) = 0;
	virtual void createGamePackage(GamePackage*, GamePackageType, OUT IGamePackageHandler**) = 0;
};

END_NS
#endif