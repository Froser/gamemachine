#include "stdafx.h"
#include "gmstring.h"

size_t GMString::npos = std::string::npos;

GMString::GMString()
{

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
	d->str = c;
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
		d->wstr.append(c);
	else
		(*this) = c;
	return *this;
}

GMString& GMString::append(const char* c)
{
	D(d);
	if (d->type == Data::MuiltBytes)
		d->str.append(c);
	else
		(*this) = c;
	return *this;
}

std::string GMString::toStdString() const
{
	D(d);
	if (d->type == Data::MuiltBytes)
		return d->str;

	std::string result;
	result.resize(d->wstr.size());
	size_t n;
	wcstombs_s(&n, &result[0], result.size(), d->wstr.data(), MB_CUR_MAX);
	return result;
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
	return d->str.substr(start, count);
}

std::wstring GMString::toStdWString() const
{
	D(d);
	if (d->type == Data::WideChars)
		return d->wstr;

	std::wstring result;
	result.resize(d->str.size());
	size_t n;
	mbstowcs_s(&n, &result[0], result.size(), d->str.data(), MB_CUR_MAX);
	return result;
}

size_t GMString::length() const
{
	D(d);
	if (d->type == Data::WideChars)
		return d->wstr.length();
	return d->str.length();
}

void GMString::copyString(void *dest)
{
	D(d);
	if (d->type == Data::WideChars)
		wcscpy_s((wchar_t*)dest, length(), d->wstr.data());
	else
		strcpy_s((char*)dest, length(), d->str.data());
}