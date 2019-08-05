#include "stdafx.h"
#include "gmfontmetrics.h"
#include "gmtypoengine.h"

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMFontMetrics)
{
	bool insetEngine = false;
	ITypoEngine* engine = nullptr;
	const IRenderContext* context = nullptr;
	GMFontMetrics::FontAttributes font;

	ITypoEngine* getTypoEngine();
};

ITypoEngine* GMFontMetricsPrivate::getTypoEngine()
{
	if (!engine)
		engine = new GMTypoEngine(context);

	return engine;
}

GMFontMetrics::GMFontMetrics(const FontAttributes& font, const IRenderContext* context)
{
	GM_CREATE_DATA();
	D(d);
	d->font = font;
	d->context = context;
}

GMFontMetrics::GMFontMetrics(const FontAttributes& font, const IRenderContext* context, ITypoEngine* engine)
	: GMFontMetrics(font, context)
{
	D(d);
	if (engine)
	{
		d->engine = engine;
		d->insetEngine = true;
	}
}

GMFontMetrics::~GMFontMetrics()
{
	D(d);
	if (d->engine)
		d->engine->destroy();
}

GMRect GMFontMetrics::boundingRect(const GMString& text)
{
	if (text.isEmpty())
		return GMRect();

	D(d);
	ITypoEngine* engine = d->getTypoEngine();
	engine->setFont(d->font.font);
	engine->setLineHeight(0);
	engine->setFontSize(d->font.fontSize);
	GMTypoOptions options;
	constexpr GMint32 enoughLong = 1024 * 10;
	constexpr GMint32 enoughHigh = 1024 * 10;
	options.typoArea = { 0, 0, enoughLong, enoughHigh };
	options.newline = false;
	options.plainText = true;
	auto iter = engine->begin(text, options);
	// 最后一个结果是结尾符，所以获取倒数第2个结果
	const GMTypoResult& typoResult = *(engine->end() - 1);
	GMTypoResult result = typoResult;
	return { 0, 0, (GMint32) (result.x + result.advance + result.bearingX), (GMint32) result.height };
}

END_NS