#ifndef __GMFONTMETRICS_H__
#define __GMFONTMETRICS_H__
#include <gmcommon.h>
BEGIN_NS

struct ITypoEngine;
GM_PRIVATE_CLASS(GMFontMetrics);
class GM_EXPORT GMFontMetrics
{
	GM_DECLARE_PRIVATE(GMFontMetrics)
	GM_DISABLE_COPY_ASSIGN(GMFontMetrics)

public:
	struct FontAttributes
	{
		GMFontSizePt fontSize = 32;
		GMFontHandle font = 0;
	};

public:
	GMFontMetrics(const FontAttributes& font, const IRenderContext* context);
	GMFontMetrics(const FontAttributes& font, const IRenderContext* context, ITypoEngine* engine);
	~GMFontMetrics();

public:
	GMRect boundingRect(const GMString& text);
};

END_NS
#endif