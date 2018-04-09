#ifndef __GMSTRING_H__
#define __GMSTRING_H__
#include <defines.h>
#include <gmobject.h>
#include <string>
#include "assert.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMString)
{
	std::wstring data;
};

static inline GMString operator +(const GMString& left, const GMString& right);

//! 表示GameMachine中字符的类
/*!
  所有的字符，在GMString中以宽字节来存储，默认采用UTF-16编码保存，与Windows一致。<br>
  如果传入的是一个窄字符，GMString将认为它，则转换为UTF-16宽字符。<br>
  任何时候都要尽量使用宽字符，以提高效率。如果不是UTF-16编码，则应该使用UTF-8编码。
*/
class GMString
{
	DECLARE_PRIVATE_NGO(GMString)

	friend inline GMString operator +(const GMString& left, const GMString& right);

public:
	static size_t npos;

public:
	//! 初始化一个空白的字符串。
	GMString() = default;

	//! 从另外一个字符串中构造一个一模一样的字符串。
	/*!
	  \param str 字符串源。
	*/
	GMString(const GMString& str);

	//! 从另外一个右值字符串中构造一个字符串。
	/*!
	  \param str 字符串源。
	*/
	GMString(GMString&& str) noexcept;

	//! 从一个C风格字符串中构造一个字符串。
	/*!
	  在内部，GMString将会把char按照UTF-8的方式，转化为GMwchar。
	  \param c 字符串源。
	*/
	GMString(const char* c);

	//! 从一个C风格的宽字符串中构造一个字符串。
	/*!
	  \param c 字符串源。
	*/
	GMString(const GMwchar* c);

	//! 从一个string标准库中构造一个字符串。
	/*!
	  在内部，GMString将会把std::string按照UTF-8的方式，转化为std::wstring，然后保存下来。
	  \param str 字符串源。
	*/
	GMString(const std::string& str);

	//! 从一个wstring标准库中构造一个字符串。
	/*!
	  \param str 字符串源。
	*/
	GMString(const std::wstring& str);

	//! 从一个C风格字符中构造一个字符串。
	/*!
	  在内部，GMString将会把char按照UTF-8的方式，转化为GMwchar。
	  \param ch 字符源。
	*/
	GMString(char ch);

	//! 从一个C风格的宽字符中构造一个字符串。
	/*!
	  \param ch 字符源。
	*/
	GMString(GMwchar ch);

	//! 从一个浮点数中构造一个字符串。
	/*!
	  \param f 浮点数源。如f为5.f的时候，GMString值为"5.000000"。
	*/
	explicit GMString(const GMfloat f);

	//! 从一个整型数中构造一个字符串。
	/*!
	  \param i 整型数源。如i为1的时候，GMString值为"1"。
	*/
	explicit GMString(const GMint i);

public:
	//! 判断此字符串和另外的字符串是否完全一样。
	/*!
	  此判断是基于字节的，和地域、语言无关。
	  \param str 待比较的字符串。
	  \return 字符串是否相同。
	*/
	bool operator == (const GMString& str) const
	{
		D(d);
		return d->data == str.toStdWString();
	}

	//! 判断此字符串和另外的字符串是否不同。
	/*!
	  此判断是基于字节的，和地域、语言无关。
	  \param str 待比较的字符串。
	  \return 字符串是否不同。
	*/
	bool operator != (const GMString& str) const
	{
		return !((*this) == str);
	}

	//! 判断此字符串是否小于另外一个字符串。
	/*!
	  此判断是基于字节的，和地域、语言无关。
	  \param str 待比较的字符串。
	  \return 是否小于另外一个字符串。
	*/
	bool operator < (const GMString& str) const
	{
		D(d);
		return d->data < str.toStdWString();
	}

	//! 将一个字符串拷贝赋值过来。
	/*!
	  \param str 目标字符串。
	  \return 此字符串自身的引用。
	*/
	GMString& operator = (const GMString& str)
	{
		assign(str);
		return *this;
	}

	//! 将一个字符串移动赋值过来。
	/*!
	  \param str 目标字符串。
	  \return 此字符串自身的引用。
	*/
	GMString& operator = (GMString&& s) noexcept
	{
		D(d);
		using namespace std;
		swap(d->data, s.data()->data);
		return *this;
	}

	//! 将一个字符串追加到此字符串末尾。
	/*!
	  \param str 目标字符串。
	  \return 此字符串自身的引用。
	  \sa append()
	*/
	GMString& operator += (const GMString& str)
	{
		*this = *this + str;
		return *this;
	}

	//! 将一个字符串追加到此字符串末尾。
	/*!
	\param str 目标字符串。
	\return 此字符串自身的引用。
	\sa append()
	*/
	GMString& operator += (GMwchar str)
	{
		D(d);
		d->data += str;
		return *this;
	}

	//! 将一个C风格字符串拷贝赋值过来。
	/*!
	  \param str 目标字符串。
	  \return 此字符串自身的引用。
	*/
	GMString& operator = (const char* str)
	{
		return this->operator=(GMString(str));
	}

	//! 将一个C风格宽字符串拷贝赋值过来。
	/*!
	  \param str 目标字符串。
	  \return 此字符串自身的引用。
	*/
	GMString& operator = (const GMwchar* str)
	{
		return this->operator=(GMString(str));
	}

	//! 取字符串的第i个，返回字符的拷贝。
	/*!
	  由于得到的是字符的拷贝，是一个右值，所以不要去修改它。
	  \return 返回指定位置的字符。
	*/
	char operator[](GMuint i) const;

	//! 获取C风格的字符串。
	/*!
	  \return C风格字符串。
	*/
	const GMwchar* c_str() const
	{
		D(d);
		return d->data.c_str();
	}

	//! 获取字符串长度。
	/*!
	  字符串的长度即字符数，而不是字节数。
	  \return 字符串长度。
	*/
	GMlong length() const
	{
		D(d);
		return d->data.length();
	}

	//! 判断字符串是否为空。
	/*!
	  任何时候，都要用此方法来代替length()==0，因为此方法有稳定的复杂度。
	  \return 字符串是否为空。
	*/
	bool isEmpty() const
	{
		D(d);
		return d->data.empty();
	}

	//! 将一个字符串追加到此字符串末尾。
	/*!
	  \param str 目标字符串。
	  \return 此字符串自身的引用。
	*/
	GMString& append(const GMString& str)
	{
		D(d);
		*this += str;
		return *this;
	}

	//! 将一个C风格字符串追加到此字符串末尾。
	/*!
	  \param c 目标字符串。
	  \return 此字符串自身的引用。
	*/
	GMString& append(const GMwchar* c);

	//! 将一个C风格字符串追加到此字符串末尾。
	/*!
	\param c 目标字符串。
	\return 此字符串自身的引用。
	*/
	GMString& append(const char* c);

	//! 清除一个字符串中的所有字符。
	void clear()
	{
		D(d);
		d->data.clear();
	}

	//! 为字符串预先分配空间。
	void reserve(GMint size)
	{
		D(d);
		d->data.reserve(size);
	}

public:
	size_t findLastOf(GMwchar c) const;
	size_t findLastOf(char c) const;
	GMString substr(GMint start, GMint count) const;
	const std::wstring& toStdWString() const;
	const std::string toStdString() const;
	GMString replace(const GMString& oldValue, const GMString& newValue);

private:
	void assign(const GMString& s);

	// 提供一些原始的字符串方法
public:
	static size_t countOfCharacters(const char* str)
	{
		return strlen(str);
	}

	static size_t countOfCharacters(const GMwchar* str)
	{
		return wcslen(str);
	}

	static void stringCopy(char* dest, size_t cchDest, const char* source);
	static void stringCopy(GMwchar* dest, size_t cchDest, const GMwchar* source);
	template <typename CharType, size_t ArraySize>
	static void stringCopy(CharType (&dest)[ArraySize], const CharType* source)
	{
		stringCopy(dest, ArraySize, source);
	}

	static void stringCopyN(char* dest, size_t cchDest, const char* source, GMint count)
	{
#if GM_MSVC
		strncpy_s(dest, cchDest, source, count);
#else
		strncpy(dest, source, count);
#endif
	}

	static void stringCopyN(GMwchar* dest, size_t cchDest, const GMwchar* source, GMint count)
	{
#if GM_MSVC
		wcsncpy_s(dest, cchDest, source, count);
#else
		wcsncpy(dest, source, count);
#endif
	}

	static void stringCat(char* dest, size_t cchDest, const char* source);
	static void stringCat(GMwchar* dest, size_t cchDest, const GMwchar* source);
	template <typename CharType, size_t ArraySize>
	static void stringCat(CharType(&dest)[ArraySize], const CharType* source)
	{
		stringCat(dest, ArraySize, source);
	}

	static bool stringEquals(const char* str1, const char* str2)
	{
		return !strcmp(str1, str2);
	}

	static bool stringEquals(const GMwchar* str1, const GMwchar* str2)
	{
		return !wcscmp(str1, str2);
	}

	static GMfloat parseFloat(const GMString& i, bool* ok = nullptr);
	static GMint parseInt(const GMString& i, bool* ok = nullptr);
};

struct GMStringHashFunctor
{
	size_t operator()(const GMString& str) const
	{
		return std::hash_value(str.toStdWString());
	}
};

inline GMString operator +(const GMString& left, const GMString& right)
{
	return left.data()->data + right.data()->data;
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


//Scanner
typedef std::function<bool(GMwchar)> CharPredicate;
GM_PRIVATE_OBJECT(GMScanner)
{
	GMString buf;
	const GMwchar* p = nullptr;
	bool skipSame;
	CharPredicate predicate;
	bool valid;
};

class GMScanner : public GMObject
{
	DECLARE_PRIVATE(GMScanner)

public:
	explicit GMScanner(const GMString& line);
	explicit GMScanner(const GMString& line, CharPredicate predicate);
	explicit GMScanner(const GMString& line, bool skipSame, CharPredicate predicate);

public:
	void next(GMString& out);
	void nextToTheEnd(GMString& out);
	bool nextFloat(GMfloat* out);
	bool nextInt(GMint* out);
};

END_NS
#endif
