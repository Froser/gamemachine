#ifndef __GM_UNICODETABLES_H__
#define __GM_UNICODETABLES_H__

#include "defines.h"
#include "gmchar.h"

BEGIN_NS

#define UNICODE_DATA_VERSION GMChar::Unicode_8_0

namespace GMUnicodeTables {
	struct Properties {
		GMushort category : 8; /* 5 used */
		GMushort direction : 8; /* 5 used */
		GMushort combiningClass : 8;
		GMushort joining : 3;
		signed short digitValue : 5;
		signed short mirrorDiff : 16;
		GMushort lowerCaseSpecial : 1;
		signed short lowerCaseDiff : 15;
		GMushort upperCaseSpecial : 1;
		signed short upperCaseDiff : 15;
		GMushort titleCaseSpecial : 1;
		signed short titleCaseDiff : 15;
		GMushort caseFoldSpecial : 1;
		signed short caseFoldDiff : 15;
		GMushort unicodeVersion : 8; /* 5 used */
		GMushort nfQuickCheck : 8;
		GMushort graphemeBreakClass : 4; /* 4 used */
		GMushort wordBreakClass : 4; /* 4 used */
		GMushort sentenceBreakClass : 8; /* 4 used */
		GMushort lineBreakClass : 8; /* 6 used */
		GMushort script : 8;
	};

	const Properties * GM_FASTCALL properties(GMuint ucs4) noexcept;
	const Properties * GM_FASTCALL properties(GMushort ucs2) noexcept;

	struct LowercaseTraits
	{
		static inline signed short caseDiff(const Properties *prop)
		{
			return prop->lowerCaseDiff;
		}
		static inline bool caseSpecial(const Properties *prop)
		{
			return prop->lowerCaseSpecial;
		}
	};

	struct UppercaseTraits
	{
		static inline signed short caseDiff(const Properties *prop)
		{
			return prop->upperCaseDiff;
		}
		static inline bool caseSpecial(const Properties *prop)
		{
			return prop->upperCaseSpecial;
		}
	};

	struct TitlecaseTraits
	{
		static inline signed short caseDiff(const Properties *prop)
		{
			return prop->titleCaseDiff;
		}
		static inline bool caseSpecial(const Properties *prop)
		{
			return prop->titleCaseSpecial;
		}
	};

	struct CasefoldTraits
	{
		static inline signed short caseDiff(const Properties *prop)
		{
			return prop->caseFoldDiff;
		}
		static inline bool caseSpecial(const Properties *prop)
		{
			return prop->caseFoldSpecial;
		}
	};

	enum GraphemeBreakClass {
		GraphemeBreak_Other,
		GraphemeBreak_CR,
		GraphemeBreak_LF,
		GraphemeBreak_Control,
		GraphemeBreak_Extend,
		GraphemeBreak_RegionalIndicator,
		GraphemeBreak_Prepend,
		GraphemeBreak_SpacingMark,
		GraphemeBreak_L,
		GraphemeBreak_V,
		GraphemeBreak_T,
		GraphemeBreak_LV,
		GraphemeBreak_LVT
	};

	enum WordBreakClass {
		WordBreak_Other,
		WordBreak_CR,
		WordBreak_LF,
		WordBreak_Newline,
		WordBreak_Extend,
		WordBreak_RegionalIndicator,
		WordBreak_Katakana,
		WordBreak_HebrewLetter,
		WordBreak_ALetter,
		WordBreak_SingleQuote,
		WordBreak_DoubleQuote,
		WordBreak_MidNumLet,
		WordBreak_MidLetter,
		WordBreak_MidNum,
		WordBreak_Numeric,
		WordBreak_ExtendNumLet
	};

	enum SentenceBreakClass {
		SentenceBreak_Other,
		SentenceBreak_CR,
		SentenceBreak_LF,
		SentenceBreak_Sep,
		SentenceBreak_Extend,
		SentenceBreak_Sp,
		SentenceBreak_Lower,
		SentenceBreak_Upper,
		SentenceBreak_OLetter,
		SentenceBreak_Numeric,
		SentenceBreak_ATerm,
		SentenceBreak_SContinue,
		SentenceBreak_STerm,
		SentenceBreak_Close
	};

	// see http://www.unicode.org/reports/tr14/tr14-30.html
	// we don't use the XX and AI classes and map them to AL instead.
	enum LineBreakClass {
		LineBreak_OP, LineBreak_CL, LineBreak_CP, LineBreak_QU, LineBreak_GL,
		LineBreak_NS, LineBreak_EX, LineBreak_SY, LineBreak_IS, LineBreak_PR,
		LineBreak_PO, LineBreak_NU, LineBreak_AL, LineBreak_HL, LineBreak_ID,
		LineBreak_IN, LineBreak_HY, LineBreak_BA, LineBreak_BB, LineBreak_B2,
		LineBreak_ZW, LineBreak_CM, LineBreak_WJ, LineBreak_H2, LineBreak_H3,
		LineBreak_JL, LineBreak_JV, LineBreak_JT, LineBreak_RI, LineBreak_CB,
		LineBreak_SA, LineBreak_SG, LineBreak_SP, LineBreak_CR, LineBreak_LF,
		LineBreak_BK
	};

	GraphemeBreakClass GM_FASTCALL graphemeBreakClass(GMuint ucs4) noexcept;
	inline GraphemeBreakClass graphemeBreakClass(GMChar ch) noexcept
	{
		return graphemeBreakClass(ch.unicode());
	}

	WordBreakClass GM_FASTCALL wordBreakClass(GMuint ucs4) noexcept;
	inline WordBreakClass wordBreakClass(GMChar ch) noexcept
	{
		return wordBreakClass(ch.unicode());
	}

	SentenceBreakClass GM_FASTCALL sentenceBreakClass(GMuint ucs4) noexcept;
	inline SentenceBreakClass sentenceBreakClass(GMChar ch) noexcept
	{
		return sentenceBreakClass(ch.unicode());
	}

	LineBreakClass GM_FASTCALL lineBreakClass(GMuint ucs4) noexcept;
	inline LineBreakClass lineBreakClass(GMChar ch) noexcept
	{
		return lineBreakClass(ch.unicode());
	}

}

END_NS

#endif