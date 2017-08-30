#include "stdafx.h"
#include "gmstring.h"

size_t GMString::npos = std::string::npos;

GMString::GMString()
{
	D(d);
	d->type = Data::WideChars;
}

GMString::GMString(const GMString& s)
{
	*this = s;
}

GMString::GMString(GMString&& s) noexcept
{
	*this = std::move(s);
}

GMString::GMString(const char c)
{
	D(d);
	d->type = Data::MuiltBytes;
	d->wstr = c;
	d->str.clear();
}

GMString::GMString(const GMWchar c)
{
	D(d);
	d->type = Data::WideChars;
	d->wstr = c;
	d->wstr.clear();
}

GMString::GMString(const char* c)
{
	D(d);
	d->type = Data::MuiltBytes;
	d->str = c;
	d->wstr.clear();
}

GMString::GMString(const GMWchar* c)
{
	D(d);
	d->type = Data::WideChars;
	d->wstr = c;
	d->str.clear();
}

GMString::GMString(const std::string& s)
{
	D(d);
	d->type = Data::MuiltBytes;
	d->str = s;
	d->wstr.clear();
}

GMString::GMString(const std::wstring& s)
{
	D(d);
	d->type = Data::WideChars;
	d->wstr = s;
	d->str.clear();
}

GMString::GMString(const GMfloat f)
{
	D(d);
	d->type = Data::WideChars;
	GMWchar buffer[128] = { 0 };
	swprintf_s(buffer, _L("%f"), f);
	d->wstr = buffer;
}

GMString::GMString(const GMint i)
{
	D(d);
	d->type = Data::WideChars;
	GMWchar buffer[128] = { 0 };
	swprintf_s(buffer, _L("%i"), i);
	d->wstr = buffer;
}

bool GMString::operator == (const GMString& str) const
{
	D(d);
	if (d->type == Data::WideChars)
		return d->wstr == str.toStdWString();
	return d->str == str.toStdString();
}

bool GMString::operator != (const GMString& str) const
{
	D(d);
	if (d->type == Data::WideChars)
		return d->wstr != str.toStdWString();
	return d->str != str.toStdString();
}

bool GMString::operator < (const GMString& str) const
{
	D(d);
	if (d->type == Data::WideChars)
		return d->wstr < str.toStdWString();
	return d->str < str.toStdString();
}

GMString& GMString::operator = (const GMString& str)
{
	D(d);
	d->type = str.data()->type;
	if (d->type == Data::WideChars)
		d->wstr = str.data()->wstr;
	else
		d->str = str.data()->str;
	return *this;
}

GMString& GMString::operator = (GMString&& str) noexcept
{
	D(d);
	d->type = str.data()->type;
	if (d->type == Data::WideChars)
		d->wstr.swap(str.data()->wstr);
	else
		d->str.swap(str.data()->str);
	return *this;
}

GMString& GMString::operator = (const char* str)
{
	return this->operator=(GMString(str));
}

GMString& GMString::operator = (const GMWchar* str)
{
	return this->operator=(GMString(str));
}

GMString& GMString::append(const GMWchar* c)
{
	D(d);
	if (d->type == Data::WideChars)
	{
		d->wstr.append(c);
	}
	else
	{
		GMString temp = toStdWString();
		(*this) = temp.append(c);
	}
	return *this;
}

GMString& GMString::append(const char* c)
{
	D(d);
	if (d->type == Data::MuiltBytes)
	{
		d->str.append(c);
	}
	else
	{
		GMString temp = toStdString();
		(*this) = temp.append(c);
	}
	return *this;
}

std::string GMString::toStdString() const
{
	D(d);
	if (d->type == Data::MuiltBytes)
		return d->str;

	d->bufString.resize(d->wstr.size());
#if _WINDOWS
	WideCharToMultiByte(CP_ACP, 0, d->wstr.data(), -1, (char*)d->bufString.data(), d->wstr.size(), 0, FALSE);
#else
#	error
#endif
	return d->bufString;
}

size_t GMString::findLastOf(GMWchar c) const
{
	D(d);
	if (d->type != Data::WideChars)
	{
		GMString temp = toStdWString();
		return temp.data()->wstr.find_last_of(c);
	}
	return d->wstr.find_last_of(c);
}

size_t GMString::findLastOf(char c) const
{
	D(d);
	if (d->type != Data::MuiltBytes)
	{
		GMString temp = toStdString();
		return temp.data()->str.find_last_of(c);
	}
	return d->str.find_last_of(c);
}

GMString GMString::substr(GMint start, GMint count) const
{
	D(d);
	if (d->type == Data::MuiltBytes)
		return d->str.substr(start, count);

	//Wide Char
	return d->wstr.substr(start, count);
}

std::wstring GMString::toStdWString() const
{
	D(d);
	if (d->type == Data::WideChars)
		return d->wstr;

	d->bufWString.resize(d->str.size());
#if _WINDOWS
	MultiByteToWideChar(CP_ACP, 0, d->str.data(), -1, (GMWchar*)d->bufWString.data(), d->str.size());
#else
#	error
#endif
	return d->bufWString;
}

size_t GMString::length() const
{
	D(d);
	if (d->type == Data::WideChars)
		return d->wstr.length();
	return d->str.length();
}

void GMString::copyString(char *dest) const
{
	D(d);
	if (d->type == Data::MuiltBytes)
	{
		strcpy_s(dest, length() + 1, d->str.c_str());
	}
	else
	{
		GMString temp = toStdString();
		strcpy_s(dest, temp.length() + 1, temp.data()->str.c_str());
	}
}

void GMString::copyString(GMWchar *dest) const
{
	D(d);
	if (d->type == Data::WideChars)
	{
		wcscpy_s(dest, length() + 1, d->wstr.c_str());
	}
	else
	{
		GMString temp = toStdString();
		wcscpy_s(dest, temp.length() + 1, temp.data()->wstr.c_str());
	}
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