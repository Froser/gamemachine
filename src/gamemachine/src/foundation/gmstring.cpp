#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include "stdafx.h"
#include "stdio.h"
#include "gmstring.h"

size_t GMString::npos = std::string::npos;

namespace
{
	GMwchar* alloc_convertMultiBytesToWideChar(const char* mbs)
	{
#if _WINDOWS
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
#if _WINDOWS
		GM_delete(c);
#endif
	}

	char* alloc_convertWideCharToMultiBytes(const GMwchar* wch)
	{
#if _WINDOWS
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
#if _WINDOWS
		GM_delete(c);
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
	GMwchar* string = alloc_convertMultiBytesToWideChar(c);
	d->data = string;
	free_wideChar(string);
}

GMString::GMString(const GMwchar* c)
{
	D(d);
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

GMString::GMString(const GMfloat f)
{
	D(d);
	d->data = std::to_wstring(f);
}

GMString::GMString(const GMint i)
{
	D(d);
	d->data = std::to_wstring(i);
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

void GMString::copyString(char *s) const
{
	D(d);
	std::string str = toStdString();
	strcpy_s(s, str.length() + 1, str.c_str());
}

void GMString::copyString(GMwchar *s) const
{
	D(d);
	wcscpy_s(s, d->data.length() * sizeof(GMwchar) + 1, d->data.c_str());
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

size_t GMString::length() const
{
	D(d);
	return d->data.length();
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