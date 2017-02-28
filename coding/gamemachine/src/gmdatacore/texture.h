#ifndef __TEXTURE_H__
#define __TEXTURE_H__
#include "common.h"
BEGIN_NS

struct TextureFrames;
struct ITexture
{
	virtual ~ITexture();
	virtual void drawTexture(TextureFrames* frames) = 0;
};

END_NS
#endif