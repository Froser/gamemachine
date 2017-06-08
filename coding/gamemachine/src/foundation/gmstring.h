#ifndef __GMSTRING_H__
#define __GMSTRING_H__
#include "common.h"
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
	StringType type;
};

// GMString
static inline GMString operator +(const GMString& left, const GMString& right);

class GMString
{
	DECLARE_PRIVATE(GMString)

	friend static inline GMString operator +(const GMString& left, const GMString& right);

public:
	GMString();
	GMString(const char* c);
	GMString(const GMwchar* c);

public:
	bool operator == (const GMString& str) const;
	bool operator != (const GMString& str) const;
	bool operator < (const GMString& str) const;

	bool operator == (const char* str) const;
	bool operator != (const char* str) const;
	bool operator < (const char* str) const;

	bool operator == (const GMwchar* str) const;
	bool operator != (const GMwchar* str) const;
	bool operator < (const GMwchar* str) const;

public:
	std::string toStdString() const;
	std::wstring toStdWString() const;

private:
	Data::StringType stringType() const;
};

static inline GMString operator +(const GMString& left, const GMString& right)
{
	if (left.stringType() == GMString::Data::WideChars)
	{
		left.data()->wstr += right.toStdWString();
	}
	else // MultiBytes
	{
		left.data()->str += right.toStdString();
	}
}

END_NS
#endif