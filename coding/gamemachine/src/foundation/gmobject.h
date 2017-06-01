#ifndef __GMOBJECT_H__
#define __GMOBJECT_H__
#include "defines.h"
#include "memory.h"

BEGIN_NS

// GameMachine采用数据和方法分离的方式，可以为一个类定义一个私有结构存储数据
template <typename T>
GM_ALIGNED_16(class) GMObjectPrivateWrapper
{
public:
	GMObjectPrivateWrapper()
		: m_data(nullptr)
	{
		m_data = new T;
	}

	~GMObjectPrivateWrapper()
	{
		if (m_data)
			delete m_data;
	}

	T* data()
	{
		return m_data;
	}

	void swap(GMObjectPrivateWrapper* other)
	{
		SWAP(m_data, other->m_data);
	}

private:
	T* m_data;
};

// 定义对齐结构体
#define GM_ALIGNED_STRUCT_FROM(name, from) GM_ALIGNED_16(struct) name : public from
#define GM_ALIGNED_STRUCT(name) GM_ALIGNED_STRUCT_FROM(name, GMAlignmentObject)
#define GM_STRUCT(name) struct name

// 对象存储
template <typename T>
GM_ALIGNED_STRUCT(GMObjectPrivateBase)
{
	GMObjectPrivateBase() : parent(nullptr) {}
	T* parent;
};

#define DECLARE_PRIVATE(className)															\
	public:																					\
		typedef className##Private Data;													\
	private:																				\
		GMObjectPrivateWrapper<className##Private> m_data;									\
		GMObjectPrivateWrapper<GMObject>* dataWrapper() {									\
			return reinterpret_cast<GMObjectPrivateWrapper<GMObject>*>(&m_data); }			\
	protected:																				\
		className##Private* data() { m_data.data()->parent = this; return m_data.data();}

// 获取私有成员
#define D(d) auto d = data()
#define D_BASE(base, d) auto d = base::data()

// 为一个对象定义private部分
#define GM_PRIVATE_OBJECT(name) class name; GM_ALIGNED_16(struct) name##Private : public GMObjectPrivateBase<name>
#define GM_PRIVATE_OBJECT_FROM(name, extends) class name; GM_ALIGNED_16(struct) name##Private : public extends##Private
#define GM_PRIVATE_NAME(name) name##Private
#define GM_PRIVATE_CONSTRUCT(name) name##Private()

// 所有GM对象的基类，如果可以用SSE指令，那么它是16字节对齐的
class GMObject : public GMAlignmentObject
{
public:
	virtual ~GMObject();

public:
	static void swapData(GMObject* a, GMObject* b);

private:
	virtual GMObjectPrivateWrapper<GMObject>* dataWrapper();
};

/* 代码规范：
  条款一：
  如果一个类是继承GMObject，那么它不能有成员变量。它的成员变量要用GM_PRIVATE_OBJECT和DECLARE_PRIVATE来声明和定义。
  这样做的好处是：
    1. 便于交往数据。两个同类型GMObject之间交换数据，只需要交换指针即可。
    2. 便于保持对齐。GM_PRIVATE_OBJECT定义出来的结构是16字节对齐的（如果需要对齐的话），保证了此结构第1个对象的地址
  肯定是16的倍数。如果要成员都是16字节对齐，应该用GM_ALIGNED_16为每个成员声明。
  对于一个对齐的对象，应该用如下方式定义：
class MyObject : public GMObject
{
    DECLARE_PRIVATE(MyObject)
}
  由于有之前的规定，MyObject不包含其他数据成员，因此DECLARE_PRIVATE定义出来的数据成员是对齐的，这样就不会产生对齐导致
的错误。

  条款二：
  如果一个结构只有方法，没有成员，可以不用继承GMObject，也不用声明为对齐。

  条款三：
  当定义对齐结构体时，结构体里面不能包含方法（构造、析构函数，以及一些operator重载除外），所有结构体用GM_ALIGNED_STRUCT
来定义，它将定义一个对齐结构体。结构体只用于存储数据，没有方法和逻辑。

  条款四：
  任何参与内核的类必须继承GMObject
*/

template <typename T>
class GMSingleton : public GMObject
{
public:
	static T& instance()
	{
		static T _instance;
		return _instance;
	}
};

// 定义一个单例类，它将生成一个Private构造函数，并将GMSingleton<>设置为其友元
#define DECLARE_SINGLETON(className) friend class GMSingleton<className>; private: className() {}

END_NS
#endif