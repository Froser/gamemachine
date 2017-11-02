#ifndef __GMGLGLYPHMANAGER_H__
#define __GMGLGLYPHMANAGER_H__
#include <gmcommon.h>
#include "gmdatacore/glyph/gmglyphmanager.h"
BEGIN_NS

class GMGLGlyphTexture;
GM_PRIVATE_OBJECT(GMGLGlyphManager)
{
	GMint cursor_u, cursor_v;
	GMfloat maxHeight;
	GMGLGlyphTexture* texture;
};

class GMGLGlyphManager : public GMGlyphManager
{
	DECLARE_PRIVATE(GMGLGlyphManager);

public:
	enum
	{
		RESOLUTION = 150,
	};

public:
	GMGLGlyphManager();
	~GMGLGlyphManager();

public:
	virtual ITexture* glyphTexture() override;

protected:
	virtual const GlyphInfo& createChar(GMWchar c, GMint fontSize) override;
};

END_NS
#endif