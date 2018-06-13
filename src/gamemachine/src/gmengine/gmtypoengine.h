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
	GMfloat lineHeight = 0;
	const GMGlyphInfo* glyph = nullptr;
	bool valid = true;
	bool newLineOrEOFSeparator = false;
};

struct ITypoEngine;

GM_PRIVATE_OBJECT(GMTypoIterator)
{
	ITypoEngine* typo = nullptr;
	GMsize_t index = 0;
};

class GMTypoIterator : public GMObject
{
	GM_DECLARE_PRIVATE(GMTypoIterator)

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
};

GM_INTERFACE(ITypoEngine)
{
	virtual GMTypoIterator begin(const GMString& literature, const GMTypoOptions& options) = 0;
	virtual GMTypoIterator end() = 0;
	virtual void setFont(GMFontHandle) = 0;

private:
	friend class GMTypoIterator;
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
	virtual ParseResult parse(REF GMwchar& ch);

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
	GMFontSizePt fontSize = 12;
	GMfloat color[3] = { 1.f, 1.f, 1.f };

	Vector<GMTypoResult> results;
};

class GMTypoEngine : public GMObject, public ITypoEngine
{
	GM_DECLARE_PRIVATE(GMTypoEngine);

public:
	GMTypoEngine(const IRenderContext* context);
	GMTypoEngine(const IRenderContext* context, AUTORELEASE GMTypoStateMachine* stateMachine);
	~GMTypoEngine();

public:
	virtual GMTypoIterator begin(const GMString& literature, const GMTypoOptions& options) override;
	virtual GMTypoIterator end() override;
	virtual void setFont(GMFontHandle font) override;

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
	bool dirty = false;
};

class GMTypoTextBuffer : public GMObject
{
	GM_DECLARE_PRIVATE(GMTypoTextBuffer);

public:
	GMTypoTextBuffer() = default;

public:
	inline void setTypoEngine(ITypoEngine* engine)
	{
		D(d);
		d->engine = engine;
	}

	void setBuffer(const GMString& string);

public:
	void setChar(GMsize_t pos, GMwchar ch);
	bool insertChar(GMsize_t pos, GMwchar ch);
	bool removeChar(GMsize_t pos);
	bool removeChars(GMsize_t startPos, GMsize_t endPos);
	GMsize_t getLength();

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