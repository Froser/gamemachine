#ifndef __GMOBJECT_H__
#define __GMOBJECT_H__
#include "defines.h"
#include "memory.h"
#include <functional>
#include <gmstring.h>

#if __APPLE__
#	include <string>
#	include <sstream>
#	include <iostream>
#endif

BEGIN_NS

//! IVirtualFunctionObject接口无其它含义，表示此类有一个虚析构函数
/*!
  继承此接口，将自动获得一个虚析构函数。
*/
struct IVirtualFunctionObject
{
	virtual ~IVirtualFunctionObject() = default;
};

// 定义对齐结构体
#define GM_ALIGNED_STRUCT_FROM(name, from) GM_ALIGNED_16(struct) name : public from
#define GM_ALIGNED_STRUCT(name) GM_ALIGNED_STRUCT_FROM(name, gm::GMAlignmentObject)

// 对象存储
template <typename T>
GM_ALIGNED_STRUCT(GMObjectPrivateBase)
{
	mutable T* __parent = nullptr;
};

template <typename T>
class GMConstructHelper
{
public:
	GMConstructHelper()
	{
		m_ref = new T();
	}

	~GMConstructHelper()
	{
		GM_delete(m_ref);
	}

	const T* data() const
	{
		return m_ref;
	}

	T* data()
	{
		return m_ref;
	}

	void swap(GMConstructHelper<T>& another) noexcept
	{
		GM_SWAP(m_ref, another.m_ref);
	}

private:
	T* m_ref;
};

class GMNotAGMObject {};

#define GM_DECLARE_PRIVATE_AND_BASE(className, base)										\
	public:																					\
		typedef base Base;																	\
		typedef className##Private Data;													\
	private:																				\
		gm::GMConstructHelper<Data> m_data;													\
	public:																					\
		inline className##Private* data() const {											\
			Data* d = const_cast<Data*>(m_data.data());										\
			d->__parent = const_cast<className*>(this); return d;}							\

#define GM_DECLARE_PRIVATE(className) GM_DECLARE_PRIVATE_AND_BASE(className, gm::GMObject)
#define GM_DECLARE_PRIVATE_NGO(className) GM_DECLARE_PRIVATE_AND_BASE(className, gm::GMNotAGMObject)

// 获取私有成员
#define D(d) auto d = data()
#define D_BASE(d, base) auto d = base::data()
#define D_OF(d, ptr) auto d = (ptr)->data()

// 为一个对象定义private部分
#define GM_PRIVATE_OBJECT(name) class name; GM_ALIGNED_16(struct) name##Private : public gm::GMObjectPrivateBase<name>
#define GM_PRIVATE_OBJECT_FROM(name, extends) class name; GM_ALIGNED_16(struct) name##Private : public extends##Private
#define GM_PRIVATE_NAME(name) name##Private
#define GM_PRIVATE_DESTRUCT(name) ~name##Private()

#define GM_DECLARE_GETTER(name, memberName, paramType) \
	public: \
	inline paramType& get##name() GM_NOEXCEPT { D(d); return d-> memberName; } \
	inline const paramType& get##name() const GM_NOEXCEPT { D(d); return d-> memberName; }

#define GM_DECLARE_SETTER(name, memberName, paramType) \
	public: \
	void set##name(const paramType & arg) { D(d); d-> memberName = arg; }

#define GM_DECLARE_PROPERTY(name, memberName, paramType) \
	GM_DECLARE_GETTER(name, memberName, paramType) \
	GM_DECLARE_SETTER(name, memberName, paramType)

#define GM_DISABLE_COPY(clsName) public: clsName(const clsName&) = delete; clsName(clsName&&) noexcept = delete;
#define GM_DISABLE_ASSIGN(clsName) public: clsName& operator =(const clsName&) = delete; clsName& operator =(clsName&&) noexcept = delete;

#define GM_ALLOW_COPY_DATA(clsName) \
	public: \
	clsName(const clsName& o) { copyData(o); } \
	clsName& operator=(const clsName& o) { D(d); copyData(o); return *this; } \
	void copyData(const clsName& another) { D(d); D_OF(d_another, &another); *d = *d_another; \
		(static_cast<Base*>(this))->copyData(static_cast<const Base&>(another)); } \
	clsName(clsName&& o) noexcept { swapData(std::move(o)); } \
	clsName& operator=(clsName&& o) noexcept { D(d); swapData(std::move(o)); return *this; } \
	void swapData(clsName&& another) noexcept { m_data.swap(another.m_data); \
		(static_cast<Base*>(this))->swapData(std::move(static_cast<Base&>(another))); }

#define GM_FRIEND_CLASS(clsName) \
	friend class clsName; \
	friend struct GM_PRIVATE_NAME(clsName);

enum class GMMetaMemberType
{
	Int,
	Float,
	Vector2,
	Vector3,
	Vector4,
	Matrix4x4,
	GMString,
	Boolean,
	Object,
};

struct GMObjectMember
{
	GMMetaMemberType type;
	size_t size;
	void* ptr;
};

using GMMeta = HashMap<GMString, GMObjectMember, GMStringHashFunctor>;

// 信号目标，表示一个GMObject被多少信号连接
class GMObject;
using GMEventCallback = std::function<void(GMObject*, GMObject*)>;
struct GMCallbackTarget
{
	GMObject* receiver;
	GMEventCallback callback;
};

typedef GMString GMSignal;

// 连接目标，表示一个GMObject连接了多少个信号
struct GMConnectionTarget
{
	GMObject* host;
	GMSignal name;
};
using GMConnectionTargets = Vector<GMConnectionTarget>;

using GMSlots = HashMap<GMSignal, Vector<GMCallbackTarget>, GMStringHashFunctor>;

#define GM_SIGNAL(signal) signal
#define GM_DECLARE_SIGNAL(signal) public: static gm::GMString GM_SIGNAL(signal);
#define GM_DEFINE_SIGNAL(signal) gm::GMString GM_SIGNAL(signal) = L#signal

#define GM_BEGIN_META_MAP \
	protected: \
	virtual bool registerMeta() override { D_BASE(db, GMObject); D(d); \
	if (db->meta.size() > 0) return true;

#define GM_BEGIN_META_MAP_FROM(base) \
	virtual bool registerMeta() override { D_BASE(db, GMObject); D(d); \
	if (db->meta.size() > 0) return true; \
	base::registerMeta();

#define GM_META(memberName, type) \
	db->meta[#memberName] = { type, sizeof(d-> memberName), &d->memberName };

#define GM_END_META_MAP \
	return true; }

// 所有GM对象的基类，如果可以用SSE指令，那么它是16字节对齐的
GM_PRIVATE_OBJECT(GMObject)
{
	GMMeta meta;
	GMSlots slots;
	GMConnectionTargets connectionTargets;
};

struct GMMessage;
//! 所有GameMachine对象的基类。
/*!
  GMObject类型只有一个指向数据成员的指针，没有其它数据成员。其数据成员指针指向一个堆上分配的数据。由于只保存一个
指向数据的指针，因此GMObject及GMObject的所有派生类禁止赋值和拷贝。<BR>
如果为一个GMObject的直接子类定义其包含的数据，可使用GM_PRIVATE_OBJECT宏来定义数据结构，并在子类中使用
GM_DECLARE_PRIVATE(子类名)来将子类的数据指针成员添加到子类中。<BR>
如果某个子类不是GMObject的直接子类，则用GM_DECLARE_PRIVATE_AND_BASE(子类名，父类名)来将数据指针添加到子类中。<BR>
使用GM_ALLOW_COPY_DATA宏来允许对象之间相互拷贝，它会生成一套左值和右值拷贝、赋值函数。<BR>
另外，GMObject的数据将会在GMObject构造时被新建，在GMObject析构时被释放。
*/
class GMObject : public IVirtualFunctionObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMObject, IVirtualFunctionObject)
	GM_DISABLE_COPY(GMObject)
	GM_DISABLE_ASSIGN(GMObject)

public:
	//! GMObject构造函数。
	/*!
	  构造一个GMObject实例。
	*/
	GMObject() = default;

	//! GMObject析构函数函数。
	/*!
	  释放GMObject，以及删除其数据成员指针，释放数据，并解绑所有信号。
	  \sa detachEvent()
	*/
	~GMObject();

public:
	//! 返回对象的元数据。
	/*!
	  对象的元数据将会在第一次返回前构造。元数据反应了此对象的数据成员中的数据类型，提供一种反射机制。
	  \return 此对象的元数据。
	*/
	const GMMeta* meta() { D(d); if (!registerMeta()) return nullptr; return &d->meta; }

	//! 为此对象绑定一个信号。
	/*!
	  为此对象绑定另外一个对象的信号，当被绑定对象的信号被触发时，执行回调函数。
	  \param sender 信号源。也就是信号的产生方。当信号源的对象的信号被触发时，执行回调函数。
	  \param signal 绑定的信号。
	  \param callback 信号被触发时执行的回调函数。
	  \sa detachEvent()
	*/
	void connect(GMObject& sender, GMSignal signal, const GMEventCallback& callback);

	//! 从某对象源中解绑一个信号。
	/*!
	  当从某对象解除绑定信号后，对象源的信号被触发后将不会通知此对象。
	  \param sender 对象源。也就是信号的产生方。
	  \param signal 需要解除的信号名。
	*/
	void disconnect(GMObject& sender, GMSignal signal);

	//! 触发一个信号。
	/*!
	  当一个信号被触发后，将会通知所有绑定了此对象此信号的所有对象，调用它们绑定的回调函数。
	  \param signal 需要触发的信号名。
	*/
	void emit(GMSignal signal);

	//! 拷贝GMObject私有数据
	/*!
	  GMObject不允许拷贝GMObject基类私有数据，因此是个空实现。<BR>
	  但是，GMObject的子类可以使用GM_ALLOW_COPY_DATA宏，允许子类调用其copyData方法，依次拷贝私有数据。
	  \param another 拷贝私有数据的目标对象，将目标对象的私有数据拷贝到此对象。
	*/
	void copyData(const GMObject& another) {}

	//! 交换GMObject私有数据
	/*!
	GMObject允许交换其私有数据。<BR>
	\param another 交换私有数据的目标对象，将目标对象的私有数据交换到此对象。
	*/
	void swapData(GMObject&& another) noexcept
	{
		m_data.swap(another.m_data);
	}

private:
	void addConnection(GMSignal signal, GMObject& receiver, GMEventCallback callback);
	void removeSignalAndConnection(GMSignal signal, GMObject& receiver);
	void removeSignal(GMSignal signal, GMObject& receiver);
	void releaseConnections();
	void addConnection(GMObject* host, GMSignal signal);
	void removeConnection(GMObject* host, GMSignal signal);

protected:
	virtual bool registerMeta() { return false; }
};

//! 进行静态转换。
/*!
如果是在调试模式下，将会返回dynamic_cast的结果。如果是在release模式下，将会返回静态转换的结果。
\param obj 需要转换的对象。
\return 转换后的对象。
*/
template <typename TargetType, typename SourceType>
inline TargetType gm_cast(SourceType obj)
{
	GM_ASSERT(dynamic_cast<TargetType>(obj));
	return static_cast<TargetType>(obj);
}

// 接口统一定义
#define GM_INTERFACE(name) struct name : public gm::IVirtualFunctionObject
#define GM_INTERFACE_FROM(name, base) struct name : public base 

template <typename T>
class GMSingleton
{
public:
	static inline T& instance()
	{
		static T _instance;
		return _instance;
	}

protected:
	GMSingleton() {}
	~GMSingleton() {}
};

// 定义一个单例类，它将生成一个Private构造函数，并将GMSingleton<>设置为其友元
#define DECLARE_SINGLETON(className) friend class GMSingleton<className>;

// 缓存类，用于存储缓存数据
struct GMBuffer
{
	GMBuffer() = default;
	~GMBuffer();

	GMBuffer(const GMBuffer& rhs);
	GMBuffer(GMBuffer&& rhs) noexcept;
	GMBuffer& operator =(GMBuffer&& rhs) noexcept;
	GMBuffer& operator =(const GMBuffer& rhs);

	void convertToStringBuffer();
	void convertToStringBufferW();
	void swap(GMBuffer& rhs);

	GMbyte* buffer = nullptr;
	GMuint size = 0;
	bool needRelease = false; // 表示是否需要手动释放
};

END_NS
#endif
