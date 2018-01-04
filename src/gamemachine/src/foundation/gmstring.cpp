#include "stdafx.h"
#include "gmstring.h"

size_t GMString::npos = std::string::npos;

namespace
{
	GMwchar* alloc_convertMultiBytesToWideChar(const char* mbs)
	{
#if GM_WINDOWS
		GMint sz = ::MultiByteToWideChar(CP_UTF8, 0, mbs, -1, nullptr, 0);
		GMwchar* data = new GMwchar[sz];
		::MultiByteToWideChar(CP_UTF8, 0, mbs, -1, data, sz);
		return data;
#else
		return nullptr;
#endif
	}

	void free_wideChar(GMwchar* c)
	{
#if GM_WINDOWS
		GM_delete(c);
#endif
	}

	char* alloc_convertWideCharToMultiBytes(const GMwchar* wch)
	{
#if GM_WINDOWS
		GMint sz = ::WideCharToMultiByte(CP_UTF8, 0, wch, -1, nullptr, 0, nullptr, nullptr);
		char* data = new char[sz];
		::WideCharToMultiByte(CP_UTF8, 0, wch, -1, data, sz, nullptr, nullptr);
		return data;
#else
		return nullptr;
#endif
	}

	void free_multibytes(char* c)
	{
#if GM_WINDOWS
		GM_delete(c);
#endif
	}

	template <typename RetType>
	GMint string_scanf(const char* buf, const char* format, RetType* ret)
	{
#if GM_MSVC
		return sscanf_s(buf, format, ret);
#else
		return sscanf(buf, format, ret);
#endif
	}

	template <typename RetType>
	GMint string_scanf(const GMwchar* buf, const GMwchar* format, RetType* ret)
	{
#if GM_MSVC
		return swscanf_s(buf, format, ret);
#else
		return swscanf(buf, format, ret);
#endif
	}
}

GMString::GMString(const GMString& s)
{
	*this = s;
}

GMString::GMString(GMString&& str) noexcept
{
	using namespace std;
	*this = move(str);
}

GMString::GMString(const char* c)
{
	D(d);
	if (c)
	{
		GMwchar* string = alloc_convertMultiBytesToWideChar(c);
		d->data = string;
		free_wideChar(string);
	}
}

GMString::GMString(const GMwchar* c)
{
	D(d);
	if (c)
		d->data = c;
}

GMString::GMString(const std::string& str)
{
	D(d);
	GMwchar* string = alloc_convertMultiBytesToWideChar(str.data());
	d->data = string;
	free_wideChar(string);
}

GMString::GMString(const std::wstring& str)
{
	D(d);
	d->data = str;
}

GMString::GMString(char ch)
{
	D(d);
	char chs[2] = { ch };
	GMwchar* string = alloc_convertMultiBytesToWideChar(chs);
	d->data = string;
	free_wideChar(string);
}

GMString::GMString(GMwchar ch)
{
	D(d);
	d->data = ch;
}

GMString::GMString(const GMfloat f)
{
	D(d);
#if GM_MSVC
	d->data = std::to_wstring(f);
#else
	GMwchar* chs = alloc_convertMultiBytesToWideChar(std::to_string(f).c_str());
	d->data = chs;
	free_wideChar(chs);
#endif
}

GMString::GMString(const GMint i)
{
	D(d);
#if GM_MSVC
	d->data = std::to_wstring(i);
#else
	GMwchar* chs = alloc_convertMultiBytesToWideChar(std::to_string(i).c_str());
	d->data = chs;
	free_wideChar(chs);
#endif
}

char GMString::operator[](GMuint i) const
{
	D(d);
	GMwchar c = d->data[i];
	GMwchar arr[2] = { c };
	char* chs = alloc_convertWideCharToMultiBytes(arr);
	char t = chs[0];
	free_multibytes(chs);
	return t;
}

GMString& GMString::append(const GMwchar* c)
{
	D(d);
	d->data += c;
	return *this;
}

void GMString::assign(const GMString& s)
{
	D(d);
	d->data = s.data()->data;
}

GMString& GMString::append(const char* c)
{
	D(d);
	GMwchar* wch = alloc_convertMultiBytesToWideChar(c);
	d->data += wch;
	free_wideChar(wch);
	return *this;
}

size_t GMString::findLastOf(GMwchar c) const
{
	D(d);
	return d->data.find_last_of(c);
}

size_t GMString::findLastOf(char c) const
{
	D(d);
	char cs[2] = { c };
	GMwchar* wch = alloc_convertMultiBytesToWideChar(cs);
	size_t idx = d->data.find_last_of(wch);
	free_wideChar(wch);
	return idx;
}

GMString GMString::substr(GMint start, GMint count) const
{
	D(d);
	return d->data.substr(start, count);
}

const std::wstring& GMString::toStdWString() const
{
	D(d);
	return d->data;
}

const std::string GMString::toStdString() const
{
	D(d);
	char* chs = alloc_convertWideCharToMultiBytes(d->data.c_str());
	std::string string(chs);
	free_multibytes(chs);
	return string;
}

GMString GMString::replace(const GMString& oldValue, const GMString& newValue)
{
	D(d);
	std::wstring _oldValue = oldValue.toStdWString();
	std::wstring _newValue = newValue.toStdWString();
	std::wstring _str = d->data;
	size_t idx = std::wstring::npos + 1;
	while ((idx = _str.find(_oldValue, _oldValue.size())) != std::wstring::npos)
	{
		_str = _str.replace(idx, _oldValue.length(), _newValue);
	}
	return _str;
}

void GMString::stringCopy(char* dest, size_t cchDest, const char* source)
{
#if GM_MSVC
	::strcpy_s(dest, cchDest, source);
#else
	strcpy(dest, source);
#endif
}

void GMString::stringCopy(GMwchar* dest, size_t cchDest, const GMwchar* source)
{
#if GM_MSVC
	::wcscpy_s(dest, cchDest, source);
#else
	wcscpy(dest, source);
#endif
}

void GMString::stringCat(char* dest, size_t cchDest, const char* source)
{
#if GM_MSVC
	::strcat_s(dest, cchDest, source);
#else
	strcat(dest, source);
#endif
}

void GMString::stringCat(GMwchar* dest, size_t cchDest, const GMwchar* source)
{
#if GM_MSVC
	::wcscat_s(dest, cchDest, source);
#else
	wcscat(dest, source);
#endif
}

GMfloat GMString::parseFloat(const GMString& i, bool* ok)
{
	GMfloat result;
	GMint c = string_scanf(i.toStdWString().c_str(), L"%f", &result);
	if (ok)
		*ok = c > 0;
	return result;
}

GMint GMString::parseInt(const GMString& i, bool* ok)
{
	GMint result;
	GMint c = string_scanf(i.toStdWString().c_str(), L"%d", &result);
	if (ok)
		*ok = c > 0;
	return result;
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
	D(d);
	d->buf = line;
	d->p = d->buf.c_str();
	d->predicate = isWhiteSpace;
	d->skipSame = true;
	d->valid = !!d->p;
}

GMScanner::GMScanner(const GMString& line, CharPredicate predicate)
{
	D(d);
	d->buf = line;
	d->p = d->buf.c_str();
	d->predicate = predicate;
	d->skipSame = true;
	d->valid = !!d->p;
}

GMScanner::GMScanner(const GMString& line, bool skipSame, CharPredicate predicate)
{
	D(d);
	d->buf = line;
	d->p = d->buf.c_str();
	d->predicate = predicate;
	d->skipSame = skipSame;
	d->valid = !!d->p;
}

void GMScanner::next(GMString& out)
{
	D(d);
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

void GMScanner::nextToTheEnd(GMString& out)
{
	D(d);
	if (!d->valid)
		return;

	while (*d->p)
	{
		d->p++;
		out += *d->p;
	}
}

bool GMScanner::nextFloat(GMfloat* out)
{
	D(d);
	if (!d->valid)
		return false;

	GMString command;
	next(command);
	if (command.isEmpty())
		return false;

	*out = GMString::parseFloat(command);
	return true;
}

bool GMScanner::nextInt(GMint* out)
{
	D(d);
	if (!d->valid)
		return false;

	GMString command;
	next(command);
	if (command.isEmpty())
		return false;
	*out = GMString::parseInt(command);
	return true;
}
