#ifndef __GMCHAR_H__
#define __GMCHAR_H__
#include "../foundation/defines.h"
#include "assert.h"
BEGIN_NS

struct GMLatin1Char
{
public:
	inline explicit GMLatin1Char(char c) noexcept : ch(c) {}
	inline char toLatin1() const noexcept { return ch; }
	inline GMushort unicode() const noexcept { return GMushort(GMbyte(ch)); }

private:
	char ch;
};


class GMChar
{
public:
	enum SpecialCharacter {
		Null = 0x0000,
		Tabulation = 0x0009,
		LineFeed = 0x000a,
		CarriageReturn = 0x000d,
		Space = 0x0020,
		Nbsp = 0x00a0,
		SoftHyphen = 0x00ad,
		ReplacementCharacter = 0xfffd,
		ObjectReplacementCharacter = 0xfffc,
		ByteOrderMark = 0xfeff,
		ByteOrderSwapped = 0xfffe,
		ParagraphSeparator = 0x2029,
		LineSeparator = 0x2028,
		LastValidCodePoint = 0x10ffff
	};

	GMChar() noexcept : ucs(0) {}
	GMChar(GMushort rc) noexcept : ucs(rc) {} // implicit
	GMChar(GMbyte c, GMbyte r) noexcept : ucs(GMushort((r << 8) | c)) {}
	GMChar(short rc) noexcept : ucs(GMushort(rc)) {} // implicit
	GMChar(GMuint rc) noexcept : ucs(GMushort(rc & 0xffff)) {}
	GMChar(int rc) noexcept : ucs(GMushort(rc & 0xffff)) {}
	GMChar(SpecialCharacter s) noexcept : ucs(GMushort(s)) {} // implicit
	GMChar(GMLatin1Char ch) noexcept : ucs(ch.unicode()) {} // implicit
	//GMChar(char16_t ch) noexcept : ucs(GMushort(ch)) {} // implicit

#if defined(_WINDOWS)
	GMChar(wchar_t ch) noexcept : ucs(GMushort(ch)) {} // implicit
#endif

	explicit GMChar(char c) noexcept : ucs(GMbyte(c)) { }
	explicit GMChar(GMbyte c) noexcept : ucs(c) { }
	// Unicode information


	enum Category
	{
		Mark_NonSpacing,          //   Mn
		Mark_SpacingCombining,    //   Mc
		Mark_Enclosing,           //   Me

		Number_DecimalDigit,      //   Nd
		Number_Letter,            //   Nl
		Number_Other,             //   No

		Separator_Space,          //   Zs
		Separator_Line,           //   Zl
		Separator_Paragraph,      //   Zp

		Other_Control,            //   Cc
		Other_Format,             //   Cf
		Other_Surrogate,          //   Cs
		Other_PrivateUse,         //   Co
		Other_NotAssigned,        //   Cn

		Letter_Uppercase,         //   Lu
		Letter_Lowercase,         //   Ll
		Letter_Titlecase,         //   Lt
		Letter_Modifier,          //   Lm
		Letter_Other,             //   Lo

		Punctuation_Connector,    //   Pc
		Punctuation_Dash,         //   Pd
		Punctuation_Open,         //   Ps
		Punctuation_Close,        //   Pe
		Punctuation_InitialQuote, //   Pi
		Punctuation_FinalQuote,   //   Pf
		Punctuation_Other,        //   Po

		Symbol_Math,              //   Sm
		Symbol_Currency,          //   Sc
		Symbol_Modifier,          //   Sk
		Symbol_Other              //   So
	};

	enum Script
	{
		Script_Unknown,
		Script_Inherited,
		Script_Common,

		Script_Latin,
		Script_Greek,
		Script_Cyrillic,
		Script_Armenian,
		Script_Hebrew,
		Script_Arabic,
		Script_Syriac,
		Script_Thaana,
		Script_Devanagari,
		Script_Bengali,
		Script_Gurmukhi,
		Script_Gujarati,
		Script_Oriya,
		Script_Tamil,
		Script_Telugu,
		Script_Kannada,
		Script_Malayalam,
		Script_Sinhala,
		Script_Thai,
		Script_Lao,
		Script_Tibetan,
		Script_Myanmar,
		Script_Georgian,
		Script_Hangul,
		Script_Ethiopic,
		Script_Cherokee,
		Script_CanadianAboriginal,
		Script_Ogham,
		Script_Runic,
		Script_Khmer,
		Script_Mongolian,
		Script_Hiragana,
		Script_Katakana,
		Script_Bopomofo,
		Script_Han,
		Script_Yi,
		Script_OldItalic,
		Script_Gothic,
		Script_Deseret,
		Script_Tagalog,
		Script_Hanunoo,
		Script_Buhid,
		Script_Tagbanwa,
		Script_Coptic,

		// Unicode 4.0 additions
		Script_Limbu,
		Script_TaiLe,
		Script_LinearB,
		Script_Ugaritic,
		Script_Shavian,
		Script_Osmanya,
		Script_Cypriot,
		Script_Braille,

		// Unicode 4.1 additions
		Script_Buginese,
		Script_NewTaiLue,
		Script_Glagolitic,
		Script_Tifinagh,
		Script_SylotiNagri,
		Script_OldPersian,
		Script_Kharoshthi,

		// Unicode 5.0 additions
		Script_Balinese,
		Script_Cuneiform,
		Script_Phoenician,
		Script_PhagsPa,
		Script_Nko,

		// Unicode 5.1 additions
		Script_Sundanese,
		Script_Lepcha,
		Script_OlChiki,
		Script_Vai,
		Script_Saurashtra,
		Script_KayahLi,
		Script_Rejang,
		Script_Lycian,
		Script_Carian,
		Script_Lydian,
		Script_Cham,

		// Unicode 5.2 additions
		Script_TaiTham,
		Script_TaiViet,
		Script_Avestan,
		Script_EgyptianHieroglyphs,
		Script_Samaritan,
		Script_Lisu,
		Script_Bamum,
		Script_Javanese,
		Script_MeeteiMayek,
		Script_ImperialAramaic,
		Script_OldSouthArabian,
		Script_InscriptionalParthian,
		Script_InscriptionalPahlavi,
		Script_OldTurkic,
		Script_Kaithi,

		// Unicode 6.0 additions
		Script_Batak,
		Script_Brahmi,
		Script_Mandaic,

		// Unicode 6.1 additions
		Script_Chakma,
		Script_MeroiticCursive,
		Script_MeroiticHieroglyphs,
		Script_Miao,
		Script_Sharada,
		Script_SoraSompeng,
		Script_Takri,

		// Unicode 7.0 additions
		Script_CaucasianAlbanian,
		Script_BassaVah,
		Script_Duployan,
		Script_Elbasan,
		Script_Grantha,
		Script_PahawhHmong,
		Script_Khojki,
		Script_LinearA,
		Script_Mahajani,
		Script_Manichaean,
		Script_MendeKikakui,
		Script_Modi,
		Script_Mro,
		Script_OldNorthArabian,
		Script_Nabataean,
		Script_Palmyrene,
		Script_PauCinHau,
		Script_OldPermic,
		Script_PsalterPahlavi,
		Script_Siddham,
		Script_Khudawadi,
		Script_Tirhuta,
		Script_WarangCiti,

		// Unicode 8.0 additions
		Script_Ahom,
		Script_AnatolianHieroglyphs,
		Script_Hatran,
		Script_Multani,
		Script_OldHungarian,
		Script_SignWriting,

		ScriptCount
	};

	enum Direction
	{
		DirL, DirR, DirEN, DirES, DirET, DirAN, DirCS, DirB, DirS, DirWS, DirON,
		DirLRE, DirLRO, DirAL, DirRLE, DirRLO, DirPDF, DirNSM, DirBN,
		DirLRI, DirRLI, DirFSI, DirPDI
	};

	enum Decomposition
	{
		NoDecomposition,
		Canonical,
		Font,
		NoBreak,
		Initial,
		Medial,
		Final,
		Isolated,
		Circle,
		Super,
		Sub,
		Vertical,
		Wide,
		Narrow,
		Small,
		Square,
		Compat,
		Fraction
	};

	enum JoiningType {
		Joining_None,
		Joining_Causing,
		Joining_Dual,
		Joining_Right,
		Joining_Left,
		Joining_Transparent
	};

	enum CombiningClass
	{
		Combining_BelowLeftAttached = 200,
		Combining_BelowAttached = 202,
		Combining_BelowRightAttached = 204,
		Combining_LeftAttached = 208,
		Combining_RightAttached = 210,
		Combining_AboveLeftAttached = 212,
		Combining_AboveAttached = 214,
		Combining_AboveRightAttached = 216,

		Combining_BelowLeft = 218,
		Combining_Below = 220,
		Combining_BelowRight = 222,
		Combining_Left = 224,
		Combining_Right = 226,
		Combining_AboveLeft = 228,
		Combining_Above = 230,
		Combining_AboveRight = 232,

		Combining_DoubleBelow = 233,
		Combining_DoubleAbove = 234,
		Combining_IotaSubscript = 240
	};

	enum UnicodeVersion {
		Unicode_Unassigned,
		Unicode_1_1,
		Unicode_2_0,
		Unicode_2_1_2,
		Unicode_3_0,
		Unicode_3_1,
		Unicode_3_2,
		Unicode_4_0,
		Unicode_4_1,
		Unicode_5_0,
		Unicode_5_1,
		Unicode_5_2,
		Unicode_6_0,
		Unicode_6_1,
		Unicode_6_2,
		Unicode_6_3,
		Unicode_7_0,
		Unicode_8_0
	};

	inline Category category() const noexcept { return GMChar::category(ucs); }
	inline Direction direction() const noexcept { return GMChar::direction(ucs); }
	inline JoiningType joiningType() const noexcept { return GMChar::joiningType(ucs); }
	inline unsigned char combiningClass() const noexcept { return GMChar::combiningClass(ucs); }

	inline GMChar mirroredChar() const noexcept { return GMChar::mirroredChar(ucs); }
	inline bool hasMirrored() const noexcept { return GMChar::hasMirrored(ucs); }

	//QString decomposition() const;
	inline Decomposition decompositionTag() const noexcept { return GMChar::decompositionTag(ucs); }

	inline int digitValue() const noexcept { return GMChar::digitValue(ucs); }
	inline GMChar toLower() const noexcept { return GMChar::toLower(ucs); }
	inline GMChar toUpper() const noexcept { return GMChar::toUpper(ucs); }
	inline GMChar toTitleCase() const noexcept { return GMChar::toTitleCase(ucs); }
	inline GMChar toCaseFolded() const noexcept { return GMChar::toCaseFolded(ucs); }

	inline Script script() const noexcept { return GMChar::script(ucs); }

	inline UnicodeVersion unicodeVersion() const noexcept { return GMChar::unicodeVersion(ucs); }

	inline char toLatin1() const noexcept { return ucs > 0xff ? '\0' : char(ucs); }
	inline GMushort unicode() const noexcept { return ucs; }
	inline GMushort &unicode() noexcept { return ucs; }
	static inline GMChar fromLatin1(char c) noexcept { return GMChar(GMushort(GMbyte(c))); }

	inline bool isNull() const noexcept { return ucs == 0; }

	inline bool isPrint() const noexcept { return GMChar::isPrint(ucs); }
	inline bool isSpace() const noexcept { return GMChar::isSpace(ucs); }
	inline bool isMark() const noexcept { return GMChar::isMark(ucs); }
	inline bool isPunct() const noexcept { return GMChar::isPunct(ucs); }
	inline bool isSymbol() const noexcept { return GMChar::isSymbol(ucs); }
	inline bool isLetter() const noexcept { return GMChar::isLetter(ucs); }
	inline bool isNumber() const noexcept { return GMChar::isNumber(ucs); }
	inline bool isLetterOrNumber() const noexcept { return GMChar::isLetterOrNumber(ucs); }
	inline bool isDigit() const noexcept { return GMChar::isDigit(ucs); }
	inline bool isLower() const noexcept { return GMChar::isLower(ucs); }
	inline bool isUpper() const noexcept { return GMChar::isUpper(ucs); }
	inline bool isTitleCase() const noexcept { return GMChar::isTitleCase(ucs); }

	inline bool isNonCharacter() const noexcept { return GMChar::isNonCharacter(ucs); }
	inline bool isHighSurrogate() const noexcept { return GMChar::isHighSurrogate(ucs); }
	inline bool isLowSurrogate() const noexcept { return GMChar::isLowSurrogate(ucs); }
	inline bool isSurrogate() const noexcept { return GMChar::isSurrogate(ucs); }

	inline GMbyte cell() const noexcept { return GMbyte(ucs & 0xff); }
	inline GMbyte row() const noexcept { return GMbyte((ucs >> 8) & 0xff); }
	inline void setCell(GMbyte acell) noexcept { ucs = GMushort((ucs & 0xff00) + acell); }
	inline void setRow(GMbyte arow) noexcept { ucs = GMushort((GMushort(arow) << 8) + (ucs & 0xff)); }

	static inline bool isNonCharacter(GMuint ucs4) noexcept
	{
		return ucs4 >= 0xfdd0 && (ucs4 <= 0xfdef || (ucs4 & 0xfffe) == 0xfffe);
	}
	static inline bool isHighSurrogate(GMuint ucs4) noexcept
	{
		return ((ucs4 & 0xfffffc00) == 0xd800);
	}
	static inline bool isLowSurrogate(GMuint ucs4) noexcept
	{
		return ((ucs4 & 0xfffffc00) == 0xdc00);
	}
	static inline bool isSurrogate(GMuint ucs4) noexcept
	{
		return (ucs4 - 0xd800u < 2048u);
	}
	static inline bool requiresSurrogates(GMuint ucs4) noexcept
	{
		return (ucs4 >= 0x10000);
	}
	static inline GMuint surrogateToUcs4(GMushort high, GMushort low) noexcept
	{
		return (GMuint(high) << 10) + low - 0x35fdc00;
	}
	static inline GMuint surrogateToUcs4(GMChar high, GMChar low) noexcept
	{
		return surrogateToUcs4(high.ucs, low.ucs);
	}
	static inline GMushort highSurrogate(GMuint ucs4) noexcept
	{
		return GMushort((ucs4 >> 10) + 0xd7c0);
	}
	static inline GMushort lowSurrogate(GMuint ucs4) noexcept
	{
		return GMushort(ucs4 % 0x400 + 0xdc00);
	}

	static Category GM_FASTCALL category(GMuint ucs4) noexcept;
	static Direction GM_FASTCALL direction(GMuint ucs4) noexcept;
	static JoiningType GM_FASTCALL joiningType(GMuint ucs4) noexcept;
	static unsigned char GM_FASTCALL combiningClass(GMuint ucs4) noexcept;

	static GMuint GM_FASTCALL mirroredChar(GMuint ucs4) noexcept;
	static bool GM_FASTCALL hasMirrored(GMuint ucs4) noexcept;

	//static QString GM_FASTCALL decomposition(GMuint ucs4);
	static Decomposition GM_FASTCALL decompositionTag(GMuint ucs4) noexcept;

	static int GM_FASTCALL digitValue(GMuint ucs4) noexcept;
	static GMuint GM_FASTCALL toLower(GMuint ucs4) noexcept;
	static GMuint GM_FASTCALL toUpper(GMuint ucs4) noexcept;
	static GMuint GM_FASTCALL toTitleCase(GMuint ucs4) noexcept;
	static GMuint GM_FASTCALL toCaseFolded(GMuint ucs4) noexcept;

	static Script GM_FASTCALL script(GMuint ucs4) noexcept;

	static UnicodeVersion GM_FASTCALL unicodeVersion(GMuint ucs4) noexcept;

	static UnicodeVersion GM_FASTCALL currentUnicodeVersion() noexcept;

	static bool GM_FASTCALL isPrint(GMuint ucs4) noexcept;
	static inline bool isSpace(GMuint ucs4) noexcept
	{
		return ucs4 == 0x20 || (ucs4 <= 0x0d && ucs4 >= 0x09)
			|| (ucs4 > 127 && (ucs4 == 0x85 || ucs4 == 0xa0 || GMChar::isSpace_helper(ucs4)));
	}
	static bool GM_FASTCALL isMark(GMuint ucs4) noexcept;
	static bool GM_FASTCALL isPunct(GMuint ucs4) noexcept;
	static bool GM_FASTCALL isSymbol(GMuint ucs4) noexcept;
	static inline bool isLetter(GMuint ucs4) noexcept
	{
		return (ucs4 >= 'A' && ucs4 <= 'z' && (ucs4 >= 'a' || ucs4 <= 'Z'))
			|| (ucs4 > 127 && GMChar::isLetter_helper(ucs4));
	}
	static inline bool isNumber(GMuint ucs4) noexcept
	{
		return (ucs4 <= '9' && ucs4 >= '0') || (ucs4 > 127 && GMChar::isNumber_helper(ucs4));
	}
	static inline bool isLetterOrNumber(GMuint ucs4) noexcept
	{
		return (ucs4 >= 'A' && ucs4 <= 'z' && (ucs4 >= 'a' || ucs4 <= 'Z'))
			|| (ucs4 >= '0' && ucs4 <= '9')
			|| (ucs4 > 127 && GMChar::isLetterOrNumber_helper(ucs4));
	}
	static inline bool isDigit(GMuint ucs4) noexcept
	{
		return (ucs4 <= '9' && ucs4 >= '0') || (ucs4 > 127 && GMChar::category(ucs4) == Number_DecimalDigit);
	}
	static inline bool isLower(GMuint ucs4) noexcept
	{
		return (ucs4 <= 'z' && ucs4 >= 'a') || (ucs4 > 127 && GMChar::category(ucs4) == Letter_Lowercase);
	}
	static inline bool isUpper(GMuint ucs4) noexcept
	{
		return (ucs4 <= 'Z' && ucs4 >= 'A') || (ucs4 > 127 && GMChar::category(ucs4) == Letter_Uppercase);
	}
	static inline bool isTitleCase(GMuint ucs4) noexcept
	{
		return ucs4 > 127 && GMChar::category(ucs4) == Letter_Titlecase;
	}

private:
	static bool GM_FASTCALL isSpace_helper(GMuint ucs4) noexcept;
	static bool GM_FASTCALL isLetter_helper(GMuint ucs4) noexcept;
	static bool GM_FASTCALL isNumber_helper(GMuint ucs4) noexcept;
	static bool GM_FASTCALL isLetterOrNumber_helper(GMuint ucs4) noexcept;

	friend bool operator==(GMChar, GMChar) noexcept;
	friend bool operator< (GMChar, GMChar) noexcept;
	GMushort ucs;
};


inline bool operator==(GMChar c1, GMChar c2) noexcept { return c1.ucs == c2.ucs; }
inline bool operator< (GMChar c1, GMChar c2) noexcept { return c1.ucs < c2.ucs; }

inline bool operator!=(GMChar c1, GMChar c2) noexcept { return !operator==(c1, c2); }
inline bool operator>=(GMChar c1, GMChar c2) noexcept { return !operator< (c1, c2); }
inline bool operator> (GMChar c1, GMChar c2) noexcept { return  operator< (c2, c1); }
inline bool operator<=(GMChar c1, GMChar c2) noexcept { return !operator< (c2, c1); }


inline bool operator==(GMChar lhs, std::nullptr_t) noexcept { return lhs.isNull(); }
inline bool operator< (GMChar, std::nullptr_t) noexcept { return false; }
inline bool operator==(std::nullptr_t, GMChar rhs) noexcept { return rhs.isNull(); }
inline bool operator< (std::nullptr_t, GMChar rhs) noexcept { return !rhs.isNull(); }

inline bool operator!=(GMChar lhs, std::nullptr_t) noexcept { return !operator==(lhs, nullptr); }
inline bool operator>=(GMChar lhs, std::nullptr_t) noexcept { return !operator< (lhs, nullptr); }
inline bool operator> (GMChar lhs, std::nullptr_t) noexcept { return  operator< (nullptr, lhs); }
inline bool operator<=(GMChar lhs, std::nullptr_t) noexcept { return !operator< (nullptr, lhs); }

inline bool operator!=(std::nullptr_t, GMChar rhs) noexcept { return !operator==(nullptr, rhs); }
inline bool operator>=(std::nullptr_t, GMChar rhs) noexcept { return !operator< (nullptr, rhs); }
inline bool operator> (std::nullptr_t, GMChar rhs) noexcept { return  operator< (rhs, nullptr); }
inline bool operator<=(std::nullptr_t, GMChar rhs) noexcept { return !operator< (rhs, nullptr); }


END_NS
#endif
