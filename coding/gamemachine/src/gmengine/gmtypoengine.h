#ifndef __GM_TYPO_H__
#define __GM_TYPO_H__
#include <gmcommon.h>
#include <gamemachine.h>

BEGIN_NS

struct GlyphInfo;
struct GMTypoResult
{
	GMint fontSize;
	GMfloat color[4];
	GMfloat x = 0;
	GMfloat y = 0;
	GMfloat lineHeight = 0;
	const GlyphInfo* glyph = nullptr;
};

struct ITypoEngine;

GM_PRIVATE_OBJECT(GMTypoIterator)
{
	ITypoEngine* typo = nullptr;
	GMint index = 0;
};

class GMTypoIterator : public GMObject
{
	DECLARE_PRIVATE(GMTypoIterator)

public:
	GMTypoIterator(ITypoEngine* typo, GMint index);

public:
	bool end();

	GMTypoResult operator*();
	bool operator==(const GMTypoIterator& rhs);
	bool operator!=(const GMTypoIterator& rhs);

	GMTypoIterator& operator ++(int)
	{
		D(d);
		++d->index;
		return *this;
	}

	GMTypoIterator& operator ++()
	{
		return (*this)++;
	}

};

struct GMTypoOptions
{
	GMint defaultFontSize = 16;

	//TODO
	GMfloat position[2] = { 0 };
};

GM_INTERFACE(ITypoEngine)
{
	virtual GMTypoIterator begin(const GMString& literature, const GMTypoOptions& options) = 0;
	virtual GMTypoIterator end() = 0;

private:
	friend class GMTypoIterator;
	virtual GMTypoResult getTypoResult(GMint index) = 0;
};

//////////////////////////////////////////////////////////////////////////
// 一个默认排版类
GM_PRIVATE_OBJECT(GMTypoEngine)
{
	GMGlyphManager* const glyphManager = GM.getGlyphManager();
	std::wstring literature;
	GMTypoOptions options;
	GMfloat lineHeight = 0;

	// 绘制状态
	GMint fontSize = 0;
};

class GMTypoEngine : public GMObject, public ITypoEngine
{
	DECLARE_PRIVATE(GMTypoEngine);

public:
	virtual GMTypoIterator begin(const GMString& literature, const GMTypoOptions& options) override;
	virtual GMTypoIterator end() override;

private:
	virtual GMTypoResult getTypoResult(GMint index) override;

};

END_NS
#endif