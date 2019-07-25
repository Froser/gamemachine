#ifndef __GM_TYPO_H__
#define __GM_TYPO_H__
#include <gmcommon.h>
#include <gamemachine.h>
#include <gmtransaction.h>

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
	GMint32 bearingX = 0;
	GMint32 bearingY = 0;
	GMint32 lineNo = 0;
	const GMGlyphInfo* glyph = nullptr;
	bool valid = true;
	bool newLineOrEOFSeparator = false;
	bool isSpace = false;
};

struct ITypoEngine;

GM_PRIVATE_CLASS(GMTypoIterator);
class GMTypoIterator
{
	GM_DECLARE_PRIVATE(GMTypoIterator)

public:
	GMTypoIterator();
	GMTypoIterator(ITypoEngine* typo, GMsize_t index);
	GMTypoIterator(const GMTypoIterator&);
	GMTypoIterator(GMTypoIterator&&) GM_NOEXCEPT;
	GMTypoIterator& operator=(const GMTypoIterator&);
	GMTypoIterator& operator=(GMTypoIterator&&) GM_NOEXCEPT;

public:
	GMTypoResult operator*();
	bool operator==(const GMTypoIterator& rhs);
	bool operator!=(const GMTypoIterator& rhs);

	GMTypoIterator& operator ++(int)
	{
		return ++(*this);
	}
	GMTypoIterator& operator ++();

	void setOffset(GMsize_t cp);
};

struct GMTypoOptions
{
	GMint32 lineSpacing = 0;
	GMRect typoArea = { 0, 0, -1, -1 }; // 排版框，排版引擎将在此框内排版
	bool center = false;
	bool newline = true;
	bool plainText = false;

	bool useCache = false;
	GMsize_t renderStart = 0;
	GMsize_t renderEnd = 0;
};

struct GMTypoResultInfo
{
	const Vector<GMTypoResult>& results;
	GMint32 lineHeight;
	GMint32 lineSpacing;
};

GM_INTERFACE(ITypoEngine)
{
	virtual GMTypoIterator begin(const GMString& literature, const GMTypoOptions& options, GMsize_t start = 0) = 0;
	virtual GMTypoIterator end() = 0;
	virtual void setFont(GMFontHandle) = 0;
	virtual void setFontSize(GMFontSizePt) = 0;
	virtual void setLineHeight(GMint32) = 0;
	virtual void createInstance(OUT ITypoEngine**) = 0;
	virtual GMTypoResultInfo getResults() = 0;
	virtual const GMTypoResultInfo getResults() const= 0;

private:
	virtual GMTypoResult getTypoResult(GMsize_t index) = 0;
};

//////////////////////////////////////////////////////////////////////////
class GMTypoEngine;
enum class GMTypoStateMachineParseState
{
	Literature,
	WaitingForCommand,
	ParsingSymbol,
};

GM_PRIVATE_CLASS(GMTypoStateMachine);
class GMTypoStateMachine : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMTypoStateMachine)
	GM_DISABLE_COPY_ASSIGN(GMTypoStateMachine)

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
	virtual void setFontSize(GMFontSizePt sz);

private:
	bool parsePair(const GMString& key, REF GMString& value);
	bool preciseParse(const GMString& name);
};

// 一个默认排版类
GM_PRIVATE_CLASS(GMTypoEngine);
class GMTypoEngine : public ITypoEngine
{
	GM_DECLARE_PRIVATE(GMTypoEngine);
	GM_DISABLE_COPY_ASSIGN(GMTypoEngine)

public:
	GMTypoEngine(const IRenderContext* context);
	~GMTypoEngine();

public:
	virtual GMTypoIterator begin(const GMString& literature, const GMTypoOptions& options, GMsize_t start) override;
	virtual GMTypoIterator end() override;
	virtual void setFont(GMFontHandle font) override;
	virtual void setFontSize(GMFontSizePt pt) override;
	virtual void setLineHeight(GMint32 lineHeight) override;
	virtual void createInstance(OUT ITypoEngine**) override;
	virtual GMTypoResultInfo getResults() override;
	virtual const GMTypoResultInfo getResults() const override;

private:
	virtual GMTypoResult getTypoResult(GMsize_t index) override;

private:
	bool isValidTypeFrame();
	void newLine();

public:
	void setColor(GMfloat rgb[3]);
};

GM_PRIVATE_CLASS(GMTypoTextBuffer);
class GMTypoTextBuffer
{
	GM_DECLARE_PRIVATE(GMTypoTextBuffer);
	GM_DISABLE_COPY_ASSIGN(GMTypoTextBuffer);

public:
	GMTypoTextBuffer();
	~GMTypoTextBuffer();

public:
	void setTypoEngine(AUTORELEASE ITypoEngine* engine);
	const ITypoEngine* getTypoEngine() const GM_NOEXCEPT;
	const GMString& getBuffer() const GM_NOEXCEPT;
	void setBuffer(const GMString& string);
	void setSize(const GMRect& rc);
	void setRenderRange(GMsize_t cpStart, GMsize_t cpEnd) GM_NOEXCEPT;
	GMsize_t getRenderStart() GM_NOEXCEPT;
	GMsize_t getRenderEnd() GM_NOEXCEPT;

public:
	void setChar(GMsize_t pos, GMwchar ch);
	bool insertChar(GMsize_t pos, GMwchar ch);
	bool insertString(GMsize_t pos, const GMString& str);
	bool removeChar(GMsize_t pos);
	bool removeChars(GMsize_t startPos, GMsize_t endPos);
	GMint32 getLength();
	GMint32 getLineHeight();
	GMwchar getChar(GMsize_t pos);

	// 排版相关
public:
	virtual bool isPlainText() GM_NOEXCEPT;
	virtual void analyze(GMint32 start);
	virtual bool CPtoX(GMint32 cp, bool trail, GMint32* x);
	virtual bool XtoCP(GMint32 x, GMint32* cp, bool* trail);
	virtual void getPriorItemPos(GMint32 cp, GMint32* prior);
	virtual void getNextItemPos(GMint32 cp, GMint32* next);

protected:
	void markDirty();
};

END_NS
#endif