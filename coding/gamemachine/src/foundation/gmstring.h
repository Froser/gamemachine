#ifndef __GMSTRING_H__
#define __GMSTRING_H__
#include "foundation/defines.h"
#include "foundation/gmobject.h"
#include <string>
BEGIN_NS

GM_PRIVATE_OBJECT(GMString)
{
	enum StringType
	{
		WideChars,
		MuiltBytes,
	};

	std::string str;
	std::wstring wstr;
	std::string bufString;
	std::wstring bufWString;
	StringType type;
};

static inline GMString operator +(const GMString& left, const GMString& right);

// 字符串类，提供Local8Bits和宽字节的支持
// 在渲染过程中不允许使用GMString，因为它会严重拖慢渲染的效率
class GMString 
{
	DECLARE_PRIVATE(GMString)

	friend static inline GMString operator +(const GMString& left, const GMString& right);

public:
	static size_t npos;

public:
	GMString();
	GMString(const GMString&);
	GMString(const char c);
	GMString(const GMWchar c);
	GMString(const char* c);
	GMString(const GMWchar* c);
	GMString(const std::string& s);
	GMString(const std::wstring& s);
	GMString(const GMfloat f);
	GMString(const GMint i);

public:
	bool operator == (const GMString& str) const;
	bool operator != (const GMString& str) const;
	bool operator < (const GMString& str) const;
	GMString& operator = (const GMString& str);
	GMString& operator = (const char* str);
	GMString& operator = (const GMWchar* str);

public:
	GMString& append(const GMWchar* c);
	GMString& append(const char* c);
	size_t findLastOf(GMWchar c) const;
	size_t findLastOf(char c) const;
	GMString substr(GMint start, GMint count) const;
	std::string toStdString() const;
	std::wstring toStdWString() const;
	size_t length() const;
	void copyString(char* dest) const;
	void copyString(GMWchar* dest) const;
};

static inline GMString operator +(const GMString& left, const GMString& right)
{
	if (left.data()->type == GMString::Data::WideChars)
	{
		return left.data()->wstr + right.toStdWString();
	}
	else // MultiBytes
	{
		return left.data()->str + right.toStdString();
	}
}

END_NS
#endif