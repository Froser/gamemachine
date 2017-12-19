#ifndef __GMOBJECT_H__
#define __GMOBJECT_H__
#include "defines.h"
#include "memory.h"
#include <functional>
#include "event_enum.h"

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
	virtual ~IVirtualFunctionObject() {}
};

// 定义对齐结构体
#define GM_ALIGNED_STRUCT_FROM(name, from) GM_ALIGNED_16(struct) name : public from
#define GM_ALIGNED_STRUCT(name) GM_ALIGNED_STRUCT_FROM(name, GMAlignmentObject)

// 对象存储
template <typename T>
GM_ALIGNED_STRUCT(GMObjectPrivateBase)
{
	mutable T* __parent = nullptr;
};

template <typename T, typename... Args>
class GMConstructHelper
{
public:
	GMConstructHelper()
	{
		m_ref = new T(Args...);
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

#define DECLARE_PRIVATE_AND_BASE(className, base)											\
	public:																					\
		typedef base Base;																	\
		typedef className##Private Data;													\
	private:																				\
		gm::GMConstructHelper<Data> m_data;													\
	public:																					\
		className##Private* data() const {													\
			Data* d = const_cast<Data*>(m_data.data());										\
			d->__parent = const_cast<className*>(this); return d;}							\
		void swapData(className& another) noexcept { m_data.swap(another.m_data); }

#define DECLARE_PRIVATE(className) DECLARE_PRIVATE_AND_BASE(className, gm::GMObject)
#define DECLARE_PRIVATE_NGO(className) DECLARE_PRIVATE_AND_BASE(className, gm::GMNotAGMObject)

// 获取私有成员
#define D(d) auto d = data()
#define D_BASE(d, base) auto d = base::data()
#define D_OF(d, ptr) auto d = (ptr)->data()

// 为一个对象定义private部分
#define GM_PRIVATE_OBJECT(name) class name; GM_ALIGNED_16(struct) name##Private : public gm::GMObjectPrivateBase<name>
#define GM_PRIVATE_OBJECT_FROM(name, extends) class name; GM_ALIGNED_16(struct) name##Private : public extends##Private
#define GM_PRIVATE_NAME(name) name##Private
#define GM_PRIVATE_CONSTRUCT(name) name##Private()
#define GM_PRIVATE_DESTRUCT(name) ~name##Private()

#define GM_DECLARE_GETTER(name, memberName, paramType) \
	public: \
	inline paramType& get##name() { D(d); return d-> memberName; } \
	inline const paramType& get##name() const { D(d); return d-> memberName; }

#define GM_DECLARE_SETTER(name, memberName, paramType) \
	public: \
	inline void set##name(const paramType & arg) { D(d); d-> memberName = arg; emitEvent(GM_SET_PROPERTY_EVENT_ENUM(name)); }

#define GM_DECLARE_PROPERTY(name, memberName, paramType) \
	GM_DECLARE_GETTER(name, memberName, paramType) \
	GM_DECLARE_SETTER(name, memberName, paramType)

#define GM_DISABLE_COPY(clsName) public: clsName(clsName&) = delete;
#define GM_DISABLE_ASSIGN(clsName) public: clsName& operator =(const clsName&) = delete;
#define GM_DEFAULT_MOVE_BEHAVIOR(clsName) \
	public: \
	clsName(clsName&& e) noexcept { gm::gmSwap(*this, e); } \
	clsName& operator=(clsName&& e) noexcept { gm::gmSwap(*this, e); return *this; }

#define GM_ALLOW_COPY_DATA(clsName) \
	public: \
	clsName(const clsName& o) { *this = o; } \
	inline clsName& operator=(const clsName& o) { D(d); *d = *o.data(); return *this; }

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

using GMMeta = Map<std::string, GMObjectMember>;

// 事件目标，表示一个GMObject被多少事件连接
class GMObject;
using GMEventCallback = std::function<void(GMObject*, GMObject*)>;
struct GMCallbackTarget
{
	GMObject* receiver;
	GMEventCallback callback;
};

typedef GMuint GMEventName;

// 连接目标，表示一个GMObject连接了多少个事件
struct GMConnectionTarget
{
	GMObject* host;
	GMEventName name;
};
using GMConnectionTargets = Vector<GMConnectionTarget>;

using GMEvents = UnorderedMap<GMEventName, Vector<GMCallbackTarget>>;

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
	GMEvents events;
	GMConnectionTargets connectionTargets;
};

struct GameMachineMessage;
//! 所有GameMachine对象的基类。
/*!
  GMObject类型只有一个指向数据成员的指针，没有其它数据成员。其数据成员指针指向一个堆上分配的数据。由于只保存一个
  指向数据的指针，因此GMObject及GMObject的所有派生类禁止赋值和拷贝。
  如果为一个GMObject的直接子类定义其包含的数据，可使用GM_PRIVATE_OBJECT宏来定义数据结构，并在子类中使用
  DECLARE_PRIVATE(子类名)来将子类的数据指针成员添加到子类中。
  如果某个子类不是GMObject的直接子类，则用DECLARE_PRIVATE_AND_BASE(子类名，父类名)来将数据指针添加到子类中。
  虽然GMObject及其子类禁止拷贝构造和赋值，但是移动构造和移动赋值是允许的。默认情况下，GMObject子类没有实现移
  动构造和移动赋值。如果要实现它，可以直接使用GM_DEFAULT_MOVE_BEHAVIOR宏。此宏会为子类添加一个移动构造和移动
  复制，它会将子类的数据以及父类的数据依次递归与右值引用中的对象进行交换。
  另外，GMObject的数据将会在GMObject构造时被新建，在GMObject析构时被释放。
*/
class GMObject : public IVirtualFunctionObject
{
	DECLARE_PRIVATE_AND_BASE(GMObject, IVirtualFunctionObject)
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
	  释放GMObject，以及删除其数据成员指针，释放数据，并解绑所有事件。
	  \sa detachEvent()
	*/
	~GMObject();

	//! GMObject移动构造函数。
	/*!
	  移动构造时，参数中的数据指针将会和本实例的数据指针进行交换。
	  \param e 需要被交换的对象。
	*/
	GMObject(GMObject&& e) noexcept;

	//! GMObject移动赋值函数。
	/*!
	  移动赋值时，参数中的数据指针将会和本实例的数据指针进行交换。
	  \param e 需要被交换的对象。
	  \return 此对象的引用。
	*/
	GMObject& operator=(GMObject&& e) noexcept;

public:
	//! 返回对象的元数据。
	/*!
	  对象的元数据将会在第一次返回前构造。元数据反应了此对象的数据成员中的数据类型，提供一种反射机制。
	  \return 此对象的元数据。
	*/
	const GMMeta* meta() { D(d); if (!registerMeta()) return nullptr; return &d->meta; }

	//! 为此对象绑定一个事件。
	/*!
	  为此对象绑定另外一个对象的事件，当被绑定对象的事件被触发时，执行回调函数。
	  \param sender 对象源。也就是事件的产生方。当事件源的对象的事件被触发时，执行回调函数。
	  \param eventName 绑定的事件名。
	  \param callback 事件被触发时执行的回调函数。
	  \sa detachEvent()
	*/
	void attachEvent(GMObject& sender, GMEventName eventName, const GMEventCallback& callback);

	//! 从某对象源中解绑一个时间。
	/*!
	  当从某对象解除绑定事件后，对象源的事件被触发后将不会通知此对象。
	  \param sender 对象源。也就是事件的产生方。
	  \param eventName 需要解除的事件名。
	*/
	void detachEvent(GMObject& sender, GMEventName eventName);

	//! 触发一个事件。
	/*!
	  当一个事件被触发后，将会通知所有绑定了此对象此事件的所有对象，调用它们绑定的回调函数。
	  \param eventName 需要触发的事件名。
	*/
	void emitEvent(GMEventName eventName);

public:
	//! 进行静态转换。
	/*!
	  如果是在调试模式下，将会返回dynamic_cast的结果。如果是在release模式下，将会返回静态转换的结果。
	  \param obj 需要转换的对象。
	  \return 转换后的对象。
	*/
	template <typename TargetType>
	static TargetType gmobject_cast(GMObject* obj)
	{
#if _DEBUG
		TargetType target = dynamic_cast<TargetType>(obj);
		GM_ASSERT(target);
		return target;
#else
		return static_cast<TargetType>(obj);
#endif
	}

private:
	void addEvent(GMEventName eventName, GMObject& receiver, const GMEventCallback& callback);
	void removeEventAndConnection(GMEventName eventName, GMObject& receiver);
	void removeEvent(GMEventName eventName, GMObject& receiver);
	void releaseEvents();
	void addConnection(GMObject* host, GMEventName eventName);
	void removeConnection(GMObject* host, GMEventName eventName);

protected:
	virtual bool registerMeta() { return false; }
};

template <typename T>
inline void gmSwap(T& a, T& b)
{
	a.swapData(b);
	gmSwap((T::Base&)a, (T::Base&)b);
}

template <>
inline void gmSwap(GMObject& a, GMObject& b)
{
	a.swapData(b);
}

// 接口统一定义
#define GM_INTERFACE(name) struct name : public gm::IVirtualFunctionObject

template <typename T>
class GMSingleton : public GMObject
{
public:
	static T& instance()
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
	GMBuffer()
		: buffer(nullptr)
		, size(0)
		, needRelease(false)
	{
	}

	~GMBuffer()
	{
		if (needRelease)
		{
			GM_delete_array(buffer);
		}
	}

	GMBuffer(GMBuffer&& rhs) noexcept
	{
		swap(rhs);
	}

	GMBuffer& operator =(GMBuffer&& rhs) noexcept
	{
		swap(rhs);
		return *this;
	}

	GMBuffer& operator =(const GMBuffer& rhs)
	{
		this->needRelease = rhs.needRelease;
		this->size = rhs.size;
		buffer = new GMbyte[this->size];
		memcpy(buffer, rhs.buffer, this->size);
		return *this;
	}

	void convertToStringBuffer()
	{
		GMbyte* newBuffer = new GMbyte[size + 1];
		memcpy(newBuffer, buffer, size);
		newBuffer[size] = 0;
		size++;
		if (needRelease && buffer)
			GM_delete_array(buffer);
		needRelease = true;
		buffer = newBuffer;
	}

	void swap(GMBuffer& rhs)
	{
		GM_SWAP(buffer, rhs.buffer);
		GM_SWAP(size, rhs.size);
		GM_SWAP(needRelease, rhs.needRelease);
	}

	GMbyte* buffer;
	GMuint size;
	bool needRelease; // 表示是否需要手动释放
};

END_NS
#endif
