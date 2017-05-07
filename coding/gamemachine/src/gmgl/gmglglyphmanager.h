#ifndef __GMGLGLYPHMANAGER_H__
#define __GMGLGLYPHMANAGER_H__
#include "common.h"
#include "gmdatacore/glyph/glyphmanager.h"
BEGIN_NS

struct GMGLGlyphManagerPrivate
{
	GMint cursor_x, cursor_y;
	GMuint textureId;
};

class GMGLGlyphManager : public GlyphManager
{
	DEFINE_PRIVATE(GMGLGlyphManager);

public:
	GMGLGlyphManager();

public:
	virtual GlyphInfo createChar(GMWChar c) override;
};

END_NS
#endif