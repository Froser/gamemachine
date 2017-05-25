#ifndef __GMOBJECT_H__
#define __GMOBJECT_H__
#include "defines.h"
#include "memory.h"

// GameMachine采用数据和方法分离的方式，可以为一个类定义一个私有结构存储数据
template <typename T>
GM_ALIGNED_16_(class) GMObjectPrivateWrapper
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

private:
	T* m_data;
};

#define DECLARE_PRIVATE_ON_HEAP(className) \
	private: \
	typedef className##Private DataType; \
	className##Private* m_data; \
	protected: \
	className##Private*& data() { return m_data; }

#define DECLARE_PRIVATE(className) \
	public: \
		typedef className##Private Data; \
	private: \
		GMObjectPrivateWrapper<className##Private> m_data; \
	protected: \
		className##Private* data() { return static_cast<className##Private*>(m_data.data()); }

// 获取私有成员
#define D(d) auto d = data()
#define D_BASE(base, d) auto d = base::data()

// 为一个对象定义private部分
#define GM_PRIVATE_OBJECT(name) GM_ALIGNED_16_(struct) name##Private : public GMAlignmentObject
#define GM_PRIVATE_OBJECT_FROM(name, extends) GM_ALIGNED_16_(struct) name##Private : public extends##Private
#define GM_PRIVATE_NAME(name) name##Private
#define GM_PRIVATE_CONSTRUCT(name) name##Private()

BEGIN_NS
// 所有GM对象的基类，如果可以用SSE指令，那么它是16字节对齐的
GM_ALIGNED_16_(class) GMObject : public GMAlignmentObject
{
public:
	virtual ~GMObject();
};

END_NS
#endif