#ifndef __GMSTRING_H__
#define __GMSTRING_H__
#include "../foundation/defines.h"
#include "../foundation/gmobject.h"
#include <string>
#include "assert.h"
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

	friend inline GMString operator +(const GMString& left, const GMString& right);

public:
	static size_t npos;

public:
	GMString();
	GMString(const GMString&);
	GMString(GMString&&) noexcept;
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
	GMString& operator = (GMString&& str) noexcept;
	GMString& operator = (const char* str);
	GMString& operator = (const GMWchar* str);

public:
	GMString& operator += (const GMString& str)
	{
		*this = *this + str;
		return *this;
	}

	char operator[](GMuint i) const
	{
		D(d);
		if (d->type == GMString::Data::MuiltBytes)
			return d->str[i];

		std::string t = toStdString();
		return t[i];
	}

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

class GMStringReader
{
	class Iterator
	{
	public:
		Iterator(const Iterator& iter) : m_src(iter.m_src), m_start(iter.m_start), m_end(iter.m_end) {}

		Iterator(const GMString& string) : m_src(string)
		{
			findNextLine();
		}

	public:
		GMString operator *();

		Iterator& operator++(int)
		{
			return ++*this;
		}

		Iterator& operator++()
		{
			findNextLine();
			return *this;
		}

		bool hasNextLine()
		{
			return !m_eof;
		}

	private:
		void findNextLine();

	private:
		const GMString& m_src;
		size_t m_start = 0, m_end = 0;
		bool m_eof = false;
	};

public:
	GMStringReader(const GMString& str) : m_string(str) {}

public:
	Iterator lineBegin();

private:
	const GMString m_string;
};
END_NS
#endif
