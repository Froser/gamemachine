#include "stdafx.h"
#include "gmstring.h"
#include "foundation/debug.h"

#if GM_UNIX
#include <wctype.h> //iswspace
#include <stdlib.h>
#endif

GMsize_t GMString::npos = std::string::npos;

namespace
{
	GMwchar* alloc_convertMultiBytesToWideChar(const char* mbs)
	{
#if GM_WINDOWS
		GMint32 sz = ::MultiByteToWideChar(CP_UTF8, 0, mbs, -1, nullptr, 0);
		GMwchar* data = new GMwchar[sz];
		::MultiByteToWideChar(CP_UTF8, 0, mbs, -1, data, sz);
		return data;
#elif GM_UNIX
		GMsize_t sz = mbstowcs(nullptr, mbs, 0) + 1;
		if (sz == 0)
		{
			gm_error(gm_dbg_wrap("Error in mbstowcs"));
			return nullptr;
		}
		GMwchar* data = new GMwchar[sz];
		mbstowcs(data, mbs, sz);
		return data;
#else
		GM_ASSERT(false);
		return nullptr;
#endif
	}

	void free_wideChar(GMwchar* c)
	{
		GM_delete(c);
	}

	char* alloc_convertWideCharToMultiBytes(const GMwchar* wch)
	{
#if GM_WINDOWS
		GMint32 sz = ::WideCharToMultiByte(CP_UTF8, 0, wch, -1, nullptr, 0, nullptr, nullptr);
		char* data = new char[sz];
		::WideCharToMultiByte(CP_UTF8, 0, wch, -1, data, sz, nullptr, nullptr);
		return data;
#elif GM_UNIX
		GMsize_t sz = wcstombs(nullptr, wch, 0) + 1;
		if (sz == 0)
		{
			gm_error(gm_dbg_wrap("Error in wcstombs"));
			return nullptr;
		}
		char* data = new char[sz];
		wcstombs(data, wch, sz);
		return data;
#else
		GM_ASSERT(false);
		return nullptr;
#endif
	}

	void free_multibytes(char* c)
	{
		GM_delete(c);
	}

	template <typename RetType>
	GMint32 string_scanf(const char* buf, const char* format, RetType* ret)
	{
#if GM_MSVC
		return sscanf_s(buf, format, ret);
#else
		return sscanf(buf, format, ret);
#endif
	}

	bool memEquals(const GMwchar* a, const GMwchar* b, GMsize_t length)
	{
		if (a == b || !length)
			return true;

		register union
		{
			const GMwchar* w;
			const GMuint32* d;
			GMsize_t value;
		} sa, sb;
		sa.w = a;
		sb.w = b;

		// check alignment
		if ((sa.value & 2) == (sb.value & 2))
		{
			// both addresses have the same alignment
			if (sa.value & 2)
			{
				// both addresses are not aligned to 4-bytes boundaries
				// compare the first character
				if (*sa.w != *sb.w)
					return false;
				--length;
				++sa.w;
				++sb.w;

				// now both addresses are 4-bytes aligned
			}

			// both addresses are 4-bytes aligned
			// do a fast 32-bit comparison
			register const GMuint32 *e = sa.d + (length >> 1);
			for (; sa.d != e; ++sa.d, ++sb.d)
			{
				if (*sa.d != *sb.d)
					return false;
			}

			// do we have a tail?
			return (length & 1) ? *sa.w == *sb.w : true;
		}
		else
		{
			// one of the addresses isn't 4-byte aligned but the other is
			register const GMwchar* e = sa.w + length;
			for (; sa.w != e; ++sa.w, ++sb.w)
			{
				if (*sa.w != *sb.w)
					return false;
			}
		}
		return true;
	}
}

GMString::GMString(const GMString& s)
{
	*this = s;
}

GMString::GMString(GMString&& str) GM_NOEXCEPT
{
	using namespace std;
	*this = move(str);
}

GMString::GMString(const char* c)
{
	D_STR(d);
	if (c)
	{
		GMwchar* string = alloc_convertMultiBytesToWideChar(c);
		d->data = string;
		free_wideChar(string);
	}
}

GMString::GMString(const GMwchar* c)
{
	D_STR(d);
	if (c)
		d->data = c;
}

GMString::GMString(const std::string& str)
{
	D_STR(d);
	GMwchar* string = alloc_convertMultiBytesToWideChar(str.data());
	d->data = string;
	free_wideChar(string);
}

GMString::GMString(const std::wstring& str)
{
	D_STR(d);
	d->data = str;
}

GMString::GMString(char ch)
{
	D_STR(d);
	char chs[2] = { ch };
	GMwchar* string = alloc_convertMultiBytesToWideChar(chs);
	d->data = string;
	free_wideChar(string);
}

GMString::GMString(GMwchar ch)
{
	D_STR(d);
	d->data = ch;
}

GMString::GMString(const GMfloat f)
{
	D_STR(d);
	d->data = std::to_wstring(f);
}

GMString::GMString(const GMint32 i)
{
	D_STR(d);
	d->data = std::to_wstring(i);
}

GMString::GMString(const GMlong i)
{
	D_STR(d);
	d->data = std::to_wstring(i);
}

char GMString::operator[](GMsize_t i) const
{
	D_STR(d);
	GMwchar c = d->data[i];
	GMwchar arr[2] = { c };
	char* chs = alloc_convertWideCharToMultiBytes(arr);
	char t = chs[0];
	free_multibytes(chs);
	return t;
}

GMwchar& GMString::operator[](GMsize_t i)
{
	D_STR(d);
	return d->data[i];
}

GMString& GMString::append(const GMwchar* c)
{
	D_STR(d);
	d->data += c;
	return *this;
}

bool GMString::startsWith(const GMString& string)
{
	D_STR(d);
	if (length() == 0)
		return string.length() == 0;
	if (string.length() > length())
		return false;
	return memEquals(d->data.data(), string.toStdWString().data(), string.length());
}

bool GMString::endsWith(const GMString& string)
{
	D_STR(d);
	if (length() == 0)
		return string.length() == 0;
	if (string.length() > length())
		return false;
	GMsize_t pos = length() - string.length();
	return memEquals(d->data.data() + pos, string.toStdWString().data(), string.length());
}

void GMString::assign(const GMString& s)
{
	D_STR(d);
	d->data = s.data()->data;
	d->hash = s.data()->hash;
	d->rehash = s.data()->rehash;
}

GMString& GMString::append(const char* c)
{
	D_STR(d);
	GMwchar* wch = alloc_convertMultiBytesToWideChar(c);
	d->data += wch;
	free_wideChar(wch);
	return *this;
}

GMsize_t GMString::findLastOf(GMwchar c) const
{
	D_STR(d);
	return d->data.find_last_of(c);
}

GMsize_t GMString::findLastOf(char c) const
{
	D_STR(d);
	char cs[2] = { c };
	GMwchar* wch = alloc_convertMultiBytesToWideChar(cs);
	GMsize_t idx = d->data.find_last_of(wch);
	free_wideChar(wch);
	return idx;
}

GMString GMString::substr(GMsize_t start, GMsize_t count) const
{
	D_STR(d);
	return d->data.substr(start, count);
}

const std::wstring& GMString::toStdWString() const
{
	D_STR(d);
	return d->data;
}

const std::string GMString::toStdString() const
{
	D_STR(d);
	char* chs = alloc_convertWideCharToMultiBytes(d->data.c_str());
	std::string string(chs);
	free_multibytes(chs);
	return string;
}

GMString GMString::replace(const GMString& oldValue, const GMString& newValue) const
{
	D_STR(d);
	std::wstring _oldValue = oldValue.toStdWString();
	std::wstring _newValue = newValue.toStdWString();
	std::wstring _str = d->data;
	GMsize_t idx = 0;
	while ((idx = _str.find(_oldValue, idx)) != std::wstring::npos)
	{
		_str = _str.replace(idx, _oldValue.length(), _newValue);
		idx += newValue.length();
	}
	return _str;
}

void GMString::stringCopy(char* dest, GMsize_t cchDest, const char* source)
{
#if GM_MSVC
	::strcpy_s(dest, cchDest, source);
#else
	strcpy(dest, source);
#endif
}

void GMString::stringCopy(GMwchar* dest, GMsize_t cchDest, const GMwchar* source)
{
#if GM_MSVC
	::wcscpy_s(dest, cchDest, source);
#else
	wcscpy(dest, source);
#endif
}

void GMString::stringCat(char* dest, GMsize_t cchDest, const char* source)
{
#if GM_MSVC
	::strcat_s(dest, cchDest, source);
#else
	strcat(dest, source);
#endif
}

void GMString::stringCat(GMwchar* dest, GMsize_t cchDest, const GMwchar* source)
{
#if GM_MSVC
	::wcscat_s(dest, cchDest, source);
#else
	wcscat(dest, source);
#endif
}

GMfloat GMString::parseFloat(const GMString& i, bool* ok)
{
	try
	{
		GMfloat v = std::stof(i.toStdWString());
		if (ok)
			*ok = true;
		return v;
	}
	catch (std::invalid_argument)
	{
		if (ok)
			*ok = false;
	}
	return 0;
}

GMint32 GMString::parseInt(const GMString& i, bool* ok)
{
	try
	{
		GMint32 v = std::stoi(i.toStdWString());
		if (ok)
			*ok = true;
		return v;
	}
	catch (std::invalid_argument)
	{
		if (ok)
			*ok = false;
	}
	return 0;
}

GMlong GMString::parseLong(const GMString& i, bool* ok)
{
	try
	{
		GMlong v = std::stol(i.toStdWString());
		if (ok)
			*ok = true;
		return v;
	}
	catch (std::invalid_argument)
	{
		if (ok)
			*ok = false;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
GMStringReader::Iterator GMStringReader::lineBegin()
{
	return GMStringReader::Iterator(m_string);
}

GMString GMStringReader::Iterator::operator *()
{
	return m_src.substr(m_start, m_end - m_start);
}

void GMStringReader::Iterator::findNextLine()
{
	m_start = m_end;
	do
	{
		if (!m_src[m_end])
		{
			m_eof = true;
			break;
		}
	} while (m_src[m_end++] != '\n');
}

//Scanner
static bool isWhiteSpace(GMwchar c)
{
	return !!iswspace(c);
}

GMScanner::GMScanner(const GMString& line)
{
	D_STR(d);
	d->buf = line;
	d->p = d->buf.c_str();
	d->predicate = isWhiteSpace;
	d->skipSame = true;
	d->valid = !!d->p;
}

GMScanner::GMScanner(const GMString& line, CharPredicate predicate)
{
	D_STR(d);
	d->buf = line;
	d->p = d->buf.c_str();
	d->predicate = predicate;
	d->skipSame = true;
	d->valid = !!d->p;
}

GMScanner::GMScanner(const GMString& line, bool skipSame, CharPredicate predicate)
{
	D_STR(d);
	d->buf = line;
	d->p = d->buf.c_str();
	d->predicate = predicate;
	d->skipSame = skipSame;
	d->valid = !!d->p;
}

void GMScanner::next(REF GMString& out)
{
	D_STR(d);
	out = L"";
	if (!d->valid)
		return;

	bool b = false;
	if (!d->p)
		return;

	while (*d->p && d->predicate(*d->p))
	{
		if (b && !d->skipSame)
		{
			d->p++;
			return;
		}
		d->p++;
		b = true;
	}

	if (!*d->p)
		return;

	do
	{
		out += *d->p;
		d->p++;
	} while (*d->p && !d->predicate(*d->p));
}

void GMScanner::nextToTheEnd(REF GMString& ref)
{
	D_STR(d);
	if (!d->valid)
		return;

	while (*d->p)
	{
		d->p++;
		ref += *d->p;
	}
}

bool GMScanner::nextFloat(REF GMfloat& ref)
{
	D_STR(d);
	if (!d->valid)
		return false;

	GMString command;
	next(command);
	if (command.isEmpty())
		return false;

	ref = GMString::parseFloat(command);
	return true;
}

bool GMScanner::nextInt(REF GMint32& ref)
{
	D_STR(d);
	if (!d->valid)
		return false;

	GMString command;
	next(command);
	if (command.isEmpty())
		return false;
	ref = GMString::parseInt(command);
	return true;
}

void GMScanner::peek(REF GMString& ref)
{
	D_STR(d);
	const GMwchar* t = d->p;
	next(ref);
	d->p = t;
}