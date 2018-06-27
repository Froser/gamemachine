#ifndef __GM_TYPO_H__
#define __GM_TYPO_H__
#include <gmcommon.h>
#include <gamemachine.h>

BEGIN_NS

struct GMGlyphInfo;
struct GMTypoResult
{
	GMFontSizePt fontSize;
	GMfloat color[4];
	GMfloat x = 0;
	GMfloat y = 0;
	GMfloat width = 0;
	GMfloat height = 0;
	GMfloat advance = 0;
	GMint lineHeight = 0;
	GMint lineNo = 0;
	const GMGlyphInfo* glyph = nullptr;
	bool valid = true;
	bool newLineOrEOFSeparator = false;
	bool isSpace = false;
};

struct ITypoEngine;

GM_PRIVATE_OBJECT(GMTypoIterator)
{
	ITypoEngine* typo = nullptr;
	GMsize_t index = 0;
};

class GMTypoIterator
{
	GM_DECLARE_PRIVATE_NGO(GMTypoIterator)

public:
	GMTypoIterator(ITypoEngine* typo, GMsize_t index);

public:
	const GMTypoResult& operator*();
	bool operator==(const GMTypoIterator& rhs);
	bool operator!=(const GMTypoIterator& rhs);

	GMTypoIterator& operator ++(int)
	{
		return ++(*this);
	}
	GMTypoIterator& operator ++();
};

struct GMTypoOptions
{
	GMFontSizePt defaultFontSize = 12;
	GMint lineSpacing = 0;
	GMRect typoArea = { 0, 0, -1, -1 }; // 排版框，排版引擎将在此框内排版
	bool center = false;
	bool newline = true;
	bool plainText = false;
};

GM_INTERFACE(ITypoEngine)
{
	virtual GMTypoIterator begin(const GMString& literature, const GMTypoOptions& options) = 0;
	virtual GMTypoIterator end() = 0;
	virtual void setFont(GMFontHandle) = 0;
	virtual GMint getLineHeight() = 0;
	virtual void createInstance(OUT ITypoEngine**) = 0;
	virtual GMTypoResult getTypoResult(GMsize_t index) = 0;
	virtual const Vector<GMTypoResult>& getResults() = 0;
};

//////////////////////////////////////////////////////////////////////////
class GMTypoEngine;
enum class GMTypoStateMachineParseState
{
	Literature,
	WaitingForCommand,
	ParsingSymbol,
};

GM_PRIVATE_OBJECT(GMTypoStateMachine)
{
	GMTypoEngine* typoEngine = nullptr;
	GMTypoStateMachineParseState state = GMTypoStateMachineParseState::Literature;
	GMString parsedSymbol;
};

class GMTypoStateMachine : public GMObject
{
	GM_DECLARE_PRIVATE(GMTypoStateMachine)

public:
	enum ParseResult
	{
		Okay,
		Newline,
		Ignore,
	};

	GMTypoStateMachine(GMTypoEngine* engine);

public:
	virtual ParseResult parse(const GMTypoOptions& options, REF GMwchar& ch);
	virtual void createInstance(GMTypoEngine* engine, OUT GMTypoStateMachine**);

protected:
	GMTypoStateMachine::ParseResult applyAttribute();

protected:
	virtual void setColor(GMfloat rgb[3]);
	virtual void setFontSize(GMint sz);

private:
	bool parsePair(const GMString& key, REF GMString& value);
	bool preciseParse(const GMString& name);
};

// 一个默认排版类
GM_PRIVATE_OBJECT(GMTypoEngine)
{
	GMFontHandle font = 0;

	GMTypoStateMachine* stateMachine = nullptr;
	const IRenderContext* context = nullptr;
	bool insetStateMachine = false;

	GMGlyphManager* glyphManager = nullptr;
	std::wstring literature;
	GMTypoOptions options;
	GMfloat lineHeight = 0;

	// 绘制状态
	GMint current_x = 0;
	GMint current_y = 0;
	GMint currentLineNo = 1;
	GMFontSizePt fontSize = 12;
	GMfloat color[3] = { 1.f, 1.f, 1.f };

	Vector<GMTypoResult> results;
};

class GMTypoEngine : public GMObject, public ITypoEngine
{
	GM_DECLARE_PRIVATE(GMTypoEngine);

public:
	GMTypoEngine(const IRenderContext* context);
	~GMTypoEngine();

public:
	virtual GMTypoIterator begin(const GMString& literature, const GMTypoOptions& options) override;
	virtual GMTypoIterator end() override;
	virtual void setFont(GMFontHandle font) override;
	virtual void createInstance(OUT ITypoEngine**) override;
	virtual GMint getLineHeight() override;

private:
	virtual GMTypoResult getTypoResult(GMsize_t index) override;

private:
	bool isValidTypeFrame();
	void newLine();

public:
	void setColor(GMfloat rgb[3]);
	void setFontSize(GMint pt);

public:
	virtual const Vector<GMTypoResult>& getResults() override;
};

GM_PRIVATE_OBJECT(GMTypoTextBuffer)
{
	ITypoEngine* engine = nullptr;
	GMString buffer;
	GMRect rc;
	bool newline = true;
	bool dirty = false;
};

class GMTypoTextBuffer : public GMObject
{
	GM_DECLARE_PRIVATE(GMTypoTextBuffer);

public:
	GMTypoTextBuffer() = default;
	~GMTypoTextBuffer();

public:
	inline void setTypoEngine(AUTORELEASE ITypoEngine* engine)
	{
		D(d);
		d->engine = engine;
	}

	inline const ITypoEngine* getTypoEngine() const GM_NOEXCEPT
	{
		D(d);
		return d->engine;
	}

	inline const GMString& getBuffer() const GM_NOEXCEPT
	{
		D(d);
		return d->buffer;
	}

	void setBuffer(const GMString& string);
	void setSize(const GMRect& rc);

public:
	void setChar(GMsize_t pos, GMwchar ch);
	bool insertChar(GMsize_t pos, GMwchar ch);
	bool insertString(GMsize_t pos, const GMString& str);
	bool removeChar(GMsize_t pos);
	bool removeChars(GMsize_t startPos, GMsize_t endPos);
	GMint getLength();
	GMint getLineHeight();

	// 排版相关
public:
	virtual void analyze();
	virtual bool CPtoX(GMint cp, bool trail, GMint* x);
	virtual bool XtoCP(GMint x, GMint* cp, bool* trail);
	virtual void getPriorItemPos(GMint cp, GMint* prior);
	virtual void getNextItemPos(GMint cp, GMint* next);

protected:
	inline void setDirty()
	{
		D(d);
		d->dirty = true;
	}
};

END_NS
#endif