#include "stdafx.h"
#include "gmstring.h"

GMString::GMString()
{

}

GMString::GMString(const char* c)
{
	D(d);
	d->type = Data::MuiltBytes;
	d->str = c;
	d->wstr.clear();
}

GMString::GMString(const GMwchar* c)
{
	D(d);
	d->type = Data::WideChars;
	d->wstr = c;
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

bool GMString::operator == (const char* str) const
{
	return *this == GMString(str);
}

bool GMString::operator != (const char* str) const
{
	return *this != GMString(str);
}

bool GMString::operator < (const char* str) const
{
	return *this < GMString(str);
}

bool GMString::operator == (const GMwchar* str) const
{
	return *this == GMString(str);
}

bool GMString::operator != (const GMwchar* str) const
{
	return *this != GMString(str);
}

bool GMString::operator < (const GMwchar* str) const
{
	return *this < GMString(str);
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

GMString::Data::StringType GMString::stringType() const
{
	D(d);
	return d->type;
}