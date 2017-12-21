#include "stdafx.h"
#include "gmchar.h"
#include "gmunicodetables.h"
#include "gmunicodetables.cpp"
#include <algorithm>

#define FLAG(x) (1 << (x))

bool GMChar::isPrint(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	const int test = FLAG(Other_Control) |
		FLAG(Other_Format) |
		FLAG(Other_Surrogate) |
		FLAG(Other_PrivateUse) |
		FLAG(Other_NotAssigned);
	return !(FLAG(gmGetProp(ucs4)->category) & test);
}

bool GM_FASTCALL GMChar::isSpace_helper(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	const int test = FLAG(Separator_Space) |
		FLAG(Separator_Line) |
		FLAG(Separator_Paragraph);
	return !!(FLAG(gmGetProp(ucs4)->category) & test);
}

bool GMChar::isMark(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	const int test = FLAG(Mark_NonSpacing) |
		FLAG(Mark_SpacingCombining) |
		FLAG(Mark_Enclosing);
	return !!(FLAG(gmGetProp(ucs4)->category) & test);
}

bool GMChar::isPunct(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	const int test = FLAG(Punctuation_Connector) |
		FLAG(Punctuation_Dash) |
		FLAG(Punctuation_Open) |
		FLAG(Punctuation_Close) |
		FLAG(Punctuation_InitialQuote) |
		FLAG(Punctuation_FinalQuote) |
		FLAG(Punctuation_Other);
	return !!(FLAG(gmGetProp(ucs4)->category) & test);
}

bool GMChar::isSymbol(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	const int test = FLAG(Symbol_Math) |
		FLAG(Symbol_Currency) |
		FLAG(Symbol_Modifier) |
		FLAG(Symbol_Other);
	return !!(FLAG(gmGetProp(ucs4)->category) & test);
}

bool GM_FASTCALL GMChar::isLetter_helper(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	const int test = FLAG(Letter_Uppercase) |
		FLAG(Letter_Lowercase) |
		FLAG(Letter_Titlecase) |
		FLAG(Letter_Modifier) |
		FLAG(Letter_Other);
	return !!(FLAG(gmGetProp(ucs4)->category) & test);
}

bool GM_FASTCALL GMChar::isNumber_helper(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	const int test = FLAG(Number_DecimalDigit) |
		FLAG(Number_Letter) |
		FLAG(Number_Other);
	return !!(FLAG(gmGetProp(ucs4)->category) & test);
}

bool GM_FASTCALL GMChar::isLetterOrNumber_helper(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	const int test = FLAG(Letter_Uppercase) |
		FLAG(Letter_Lowercase) |
		FLAG(Letter_Titlecase) |
		FLAG(Letter_Modifier) |
		FLAG(Letter_Other) |
		FLAG(Number_DecimalDigit) |
		FLAG(Number_Letter) |
		FLAG(Number_Other);
	return !!(FLAG(gmGetProp(ucs4)->category) & test);
}

int GMChar::digitValue(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return -1;
	return gmGetProp(ucs4)->digitValue;
}

GMChar::Category GMChar::category(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return GMChar::Other_NotAssigned;
	return (GMChar::Category) gmGetProp(ucs4)->category;
}

GMChar::Direction GMChar::direction(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return GMChar::DirL;
	return (GMChar::Direction) gmGetProp(ucs4)->direction;
}

GMChar::JoiningType GMChar::joiningType(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return GMChar::Joining_None;
	return GMChar::JoiningType(gmGetProp(ucs4)->joining);
}

bool GMChar::hasMirrored(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return false;
	return gmGetProp(ucs4)->mirrorDiff != 0;
}

GMuint GMChar::mirroredChar(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return ucs4;
	return ucs4 + gmGetProp(ucs4)->mirrorDiff;
}

enum {
	Hangul_SBase = 0xac00,
	Hangul_LBase = 0x1100,
	Hangul_VBase = 0x1161,
	Hangul_TBase = 0x11a7,
	Hangul_LCount = 19,
	Hangul_VCount = 21,
	Hangul_TCount = 28,
	Hangul_NCount = Hangul_VCount * Hangul_TCount,
	Hangul_SCount = Hangul_LCount * Hangul_NCount
};

// buffer has to have a length of 3. It's needed for Hangul decomposition
static const unsigned short * GM_FASTCALL decompositionHelper
(GMuint ucs4, int *length, int *tag, unsigned short *buffer)
{
	if (ucs4 >= Hangul_SBase && ucs4 < Hangul_SBase + Hangul_SCount) {
		// compute Hangul syllable decomposition as per UAX #15
		const GMuint SIndex = ucs4 - Hangul_SBase;
		buffer[0] = Hangul_LBase + SIndex / Hangul_NCount; // L
		buffer[1] = Hangul_VBase + (SIndex % Hangul_NCount) / Hangul_TCount; // V
		buffer[2] = Hangul_TBase + SIndex % Hangul_TCount; // T
		*length = buffer[2] == Hangul_TBase ? 2 : 3;
		*tag = GMChar::Canonical;
		return buffer;
	}

	const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
	if (index == 0xffff) {
		*length = 0;
		*tag = GMChar::NoDecomposition;
		return 0;
	}

	const unsigned short *decomposition = uc_decomposition_map + index;
	*tag = (*decomposition) & 0xff;
	*length = (*decomposition) >> 8;
	return decomposition + 1;
}

/*
QString GMChar::decomposition() const
{
	return GMChar::decomposition(ucs);
}

QString GMChar::decomposition(GMuint ucs4)
{
	unsigned short buffer[3];
	int length;
	int tag;
	const unsigned short *d = decompositionHelper(ucs4, &length, &tag, buffer);
	return QString(reinterpret_cast<const GMChar *>(d), length);
}
*/

GMChar::Decomposition GMChar::decompositionTag(GMuint ucs4) noexcept
{
	if (ucs4 >= Hangul_SBase && ucs4 < Hangul_SBase + Hangul_SCount)
		return GMChar::Canonical;
	const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
	if (index == 0xffff)
		return GMChar::NoDecomposition;
	return (GMChar::Decomposition)(uc_decomposition_map[index] & 0xff);
}

unsigned char GMChar::combiningClass(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return 0;
	return (unsigned char)gmGetProp(ucs4)->combiningClass;
}

GMChar::Script GMChar::script(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return GMChar::Script_Unknown;
	return (GMChar::Script) gmGetProp(ucs4)->script;
}

GMChar::UnicodeVersion GMChar::unicodeVersion(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return GMChar::Unicode_Unassigned;
	return (GMChar::UnicodeVersion) gmGetProp(ucs4)->unicodeVersion;
}

GMChar::UnicodeVersion GMChar::currentUnicodeVersion() noexcept
{
	return UNICODE_DATA_VERSION;
}

template <typename Traits, typename T>
static inline T convertCase_helper(T uc) noexcept
{
	const GMUnicodeTables::Properties *prop = gmGetProp(uc);
	if (Traits::caseSpecial(prop)) {
		const GMushort *specialCase = specialCaseMap + Traits::caseDiff(prop);
		// so far, there are no special cases beyond BMP (guaranteed by the qunicodetables generator)
		return *specialCase == 1 ? specialCase[1] : uc;
	}

	return uc + Traits::caseDiff(prop);
}

GMuint GMChar::toLower(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return ucs4;
	return convertCase_helper<GMUnicodeTables::LowercaseTraits>(ucs4);
}

GMuint GMChar::toUpper(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return ucs4;
	return convertCase_helper<GMUnicodeTables::UppercaseTraits>(ucs4);
}

GMuint GMChar::toTitleCase(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return ucs4;
	return convertCase_helper<GMUnicodeTables::TitlecaseTraits>(ucs4);
}

static inline GMuint foldCase(const GMushort *ch, const GMushort *start)
{
	GMuint ucs4 = *ch;
	if (GMChar::isLowSurrogate(ucs4) && ch > start && GMChar::isHighSurrogate(*(ch - 1)))
		ucs4 = GMChar::surrogateToUcs4(*(ch - 1), ucs4);
	return convertCase_helper<GMUnicodeTables::CasefoldTraits>(ucs4);
}

static inline GMuint foldCase(GMuint ch, GMuint &last) noexcept
{
	GMuint ucs4 = ch;
	if (GMChar::isLowSurrogate(ucs4) && GMChar::isHighSurrogate(last))
		ucs4 = GMChar::surrogateToUcs4(last, ucs4);
	last = ch;
	return convertCase_helper<GMUnicodeTables::CasefoldTraits>(ucs4);
}

static inline GMushort foldCase(GMushort ch) noexcept
{
	return convertCase_helper<GMUnicodeTables::CasefoldTraits>(ch);
}

static inline GMChar foldCase(GMChar ch) noexcept
{
	return GMChar(foldCase(ch.unicode()));
}

GMuint GMChar::toCaseFolded(GMuint ucs4) noexcept
{
	if (ucs4 > LastValidCodePoint)
		return ucs4;
	return convertCase_helper<GMUnicodeTables::CasefoldTraits>(ucs4);
}
/*
static void decomposeHelper(QString *str, bool canonical, GMChar::UnicodeVersion version, int from)
{
	int length;
	int tag;
	unsigned short buffer[3];

	QString &s = *str;

	const unsigned short *utf16 = reinterpret_cast<unsigned short *>(s.data());
	const unsigned short *uc = utf16 + s.length();
	while (uc != utf16 + from) {
		GMuint ucs4 = *(--uc);
		if (GMChar(ucs4).isLowSurrogate() && uc != utf16) {
			GMushort high = *(uc - 1);
			if (GMChar(high).isHighSurrogate()) {
				--uc;
				ucs4 = GMChar::surrogateToUcs4(high, ucs4);
			}
		}

		if (GMChar::unicodeVersion(ucs4) > version)
			continue;

		const unsigned short *d = decompositionHelper(ucs4, &length, &tag, buffer);
		if (!d || (canonical && tag != GMChar::Canonical))
			continue;

		int pos = uc - utf16;
		s.replace(pos, GMChar::requiresSurrogates(ucs4) ? 2 : 1, reinterpret_cast<const GMChar *>(d), length);
		// since the replace invalidates the pointers and we do decomposition recursive
		utf16 = reinterpret_cast<unsigned short *>(s.data());
		uc = utf16 + pos + length;
	}
}


struct UCS2Pair {
	GMushort u1;
	GMushort u2;
};

inline bool operator<(const UCS2Pair &ligature1, const UCS2Pair &ligature2)
{
	return ligature1.u1 < ligature2.u1;
}
inline bool operator<(GMushort u1, const UCS2Pair &ligature)
{
	return u1 < ligature.u1;
}
inline bool operator<(const UCS2Pair &ligature, GMushort u1)
{
	return ligature.u1 < u1;
}

struct UCS2SurrogatePair {
	UCS2Pair p1;
	UCS2Pair p2;
};

inline bool operator<(const UCS2SurrogatePair &ligature1, const UCS2SurrogatePair &ligature2)
{
	return GMChar::surrogateToUcs4(ligature1.p1.u1, ligature1.p1.u2) < GMChar::surrogateToUcs4(ligature2.p1.u1, ligature2.p1.u2);
}
inline bool operator<(GMuint u1, const UCS2SurrogatePair &ligature)
{
	return u1 < GMChar::surrogateToUcs4(ligature.p1.u1, ligature.p1.u2);
}
inline bool operator<(const UCS2SurrogatePair &ligature, GMuint u1)
{
	return GMChar::surrogateToUcs4(ligature.p1.u1, ligature.p1.u2) < u1;
}

static GMuint inline ligatureHelper(GMuint u1, GMuint u2)
{
	if (u1 >= Hangul_LBase && u1 <= Hangul_SBase + Hangul_SCount) {
		// compute Hangul syllable composition as per UAX #15
		// hangul L-V pair
		const GMuint LIndex = u1 - Hangul_LBase;
		if (LIndex < Hangul_LCount) {
			const GMuint VIndex = u2 - Hangul_VBase;
			if (VIndex < Hangul_VCount)
				return Hangul_SBase + (LIndex * Hangul_VCount + VIndex) * Hangul_TCount;
		}
		// hangul LV-T pair
		const GMuint SIndex = u1 - Hangul_SBase;
		if (SIndex < Hangul_SCount && (SIndex % Hangul_TCount) == 0) {
			const GMuint TIndex = u2 - Hangul_TBase;
			if (TIndex <= Hangul_TCount)
				return u1 + TIndex;
		}
	}

	const unsigned short index = GET_LIGATURE_INDEX(u2);
	if (index == 0xffff)
		return 0;
	const unsigned short *ligatures = uc_ligature_map + index;
	GMushort length = *ligatures++;
	if (GMChar::requiresSurrogates(u1)) {
		const UCS2SurrogatePair *data = reinterpret_cast<const UCS2SurrogatePair *>(ligatures);
		const UCS2SurrogatePair *r = std::lower_bound(data, data + length, u1);
		if (r != data + length && GMChar::surrogateToUcs4(r->p1.u1, r->p1.u2) == u1)
			return GMChar::surrogateToUcs4(r->p2.u1, r->p2.u2);
	}
	else {
		const UCS2Pair *data = reinterpret_cast<const UCS2Pair *>(ligatures);
		const UCS2Pair *r = std::lower_bound(data, data + length, GMushort(u1));
		if (r != data + length && r->u1 == GMushort(u1))
			return r->u2;
	}

	return 0;
}

static void composeHelper(QString *str, GMChar::UnicodeVersion version, int from)
{
	QString &s = *str;

	if (from < 0 || s.length() - from < 2)
		return;

	GMuint stcode = 0; // starter code point
	int starter = -1; // starter position
	int next = -1; // to prevent i == next
	int lastCombining = 255; // to prevent combining > lastCombining

	int pos = from;
	while (pos < s.length()) {
		int i = pos;
		GMuint uc = s.at(pos).unicode();
		if (GMChar(uc).isHighSurrogate() && pos < s.length() - 1) {
			GMushort low = s.at(pos + 1).unicode();
			if (GMChar(low).isLowSurrogate()) {
				uc = GMChar::surrogateToUcs4(uc, low);
				++pos;
			}
		}

		const QUnicodeTables::Properties *p = qGetProp(uc);
		if (p->unicodeVersion > version) {
			starter = -1;
			next = -1; // to prevent i == next
			lastCombining = 255; // to prevent combining > lastCombining
			++pos;
			continue;
		}

		int combining = p->combiningClass;
		if ((i == next || combining > lastCombining) && starter >= from) {
			// allowed to form ligature with S
			GMuint ligature = ligatureHelper(stcode, uc);
			if (ligature) {
				stcode = ligature;
				GMChar *d = s.data();
				// ligatureHelper() never changes planes
				if (GMChar::requiresSurrogates(ligature)) {
					d[starter] = GMChar::highSurrogate(ligature);
					d[starter + 1] = GMChar::lowSurrogate(ligature);
					s.remove(i, 2);
				}
				else {
					d[starter] = ligature;
					s.remove(i, 1);
				}
				continue;
			}
		}
		if (combining == 0) {
			starter = i;
			stcode = uc;
			next = pos + 1;
		}
		lastCombining = combining;

		++pos;
	}
}


static void canonicalOrderHelper(QString *str, GMChar::UnicodeVersion version, int from)
{
	QString &s = *str;
	const int l = s.length() - 1;

	GMuint u1, u2;
	GMushort c1, c2;

	int pos = from;
	while (pos < l) {
		int p2 = pos + 1;
		u1 = s.at(pos).unicode();
		if (GMChar(u1).isHighSurrogate()) {
			GMushort low = s.at(p2).unicode();
			if (GMChar(low).isLowSurrogate()) {
				u1 = GMChar::surrogateToUcs4(u1, low);
				if (p2 >= l)
					break;
				++p2;
			}
		}
		c1 = 0;

	advance:
		u2 = s.at(p2).unicode();
		if (GMChar(u2).isHighSurrogate() && p2 < l) {
			GMushort low = s.at(p2 + 1).unicode();
			if (GMChar(low).isLowSurrogate()) {
				u2 = GMChar::surrogateToUcs4(u2, low);
				++p2;
			}
		}

		c2 = 0;
		{
			const QUnicodeTables::Properties *p = qGetProp(u2);
			if (p->unicodeVersion <= version)
				c2 = p->combiningClass;
		}
		if (c2 == 0) {
			pos = p2 + 1;
			continue;
		}

		if (c1 == 0) {
			const QUnicodeTables::Properties *p = qGetProp(u1);
			if (p->unicodeVersion <= version)
				c1 = p->combiningClass;
		}

		if (c1 > c2) {
			GMChar *uc = s.data();
			int p = pos;
			// exchange characters
			if (!GMChar::requiresSurrogates(u2)) {
				uc[p++] = u2;
			}
			else {
				uc[p++] = GMChar::highSurrogate(u2);
				uc[p++] = GMChar::lowSurrogate(u2);
			}
			if (!GMChar::requiresSurrogates(u1)) {
				uc[p++] = u1;
			}
			else {
				uc[p++] = GMChar::highSurrogate(u1);
				uc[p++] = GMChar::lowSurrogate(u1);
			}
			if (pos > 0)
				--pos;
			if (pos > 0 && s.at(pos).isLowSurrogate())
				--pos;
		}
		else {
			++pos;
			if (GMChar::requiresSurrogates(u1))
				++pos;

			u1 = u2;
			c1 = c2; // != 0
			p2 = pos + 1;
			if (GMChar::requiresSurrogates(u1))
				++p2;
			if (p2 > l)
				break;

			goto advance;
		}
	}
}

// returns true if the text is in a desired Normalization Form already; false otherwise.
// sets lastStable to the position of the last stable code point
static bool normalizationQuickCheckHelper(QString *str, QString::NormalizationForm mode, int from, int *lastStable)
{
	Q_STATIC_ASSERT(QString::NormalizationForm_D == 0);
	Q_STATIC_ASSERT(QString::NormalizationForm_C == 1);
	Q_STATIC_ASSERT(QString::NormalizationForm_KD == 2);
	Q_STATIC_ASSERT(QString::NormalizationForm_KC == 3);

	enum { NFQC_YES = 0, NFQC_NO = 1, NFQC_MAYBE = 3 };

	const GMushort *string = reinterpret_cast<const GMushort *>(str->constData());
	int length = str->length();

	// this avoids one out of bounds check in the loop
	while (length > from && GMChar::isHighSurrogate(string[length - 1]))
		--length;

	uchar lastCombining = 0;
	for (int i = from; i < length; ++i) {
		int pos = i;
		GMuint uc = string[i];
		if (uc < 0x80) {
			// ASCII characters are stable code points
			lastCombining = 0;
			*lastStable = pos;
			continue;
		}

		if (GMChar::isHighSurrogate(uc)) {
			GMushort low = string[i + 1];
			if (!GMChar::isLowSurrogate(low)) {
				// treat surrogate like stable code point
				lastCombining = 0;
				*lastStable = pos;
				continue;
			}
			++i;
			uc = GMChar::surrogateToUcs4(uc, low);
		}

		const QUnicodeTables::Properties *p = qGetProp(uc);

		if (p->combiningClass < lastCombining && p->combiningClass > 0)
			return false;

		const uchar check = (p->nfQuickCheck >> (mode << 1)) & 0x03;
		if (check != NFQC_YES)
			return false; // ### can we quick check NFQC_MAYBE ?

		lastCombining = p->combiningClass;
		if (lastCombining == 0)
			*lastStable = pos;
	}

	if (length != str->length()) // low surrogate parts at the end of text
		*lastStable = str->length() - 1;

	return true;
}
*/