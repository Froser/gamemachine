#ifndef __GMOBJECT_H__
#define __GMOBJECT_H__
#include "defines.h"
#include "memory.h"
#include <functional>
#include <gmstring.h>
#include <linearmath.h>

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
struct GM_EXPORT IVirtualFunctionObject
{
	virtual ~IVirtualFunctionObject() = default;
};

// 对象存储
template <typename T>
GM_ALIGNED_STRUCT(GMObjectPrivateBase)
{
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

	void swap(GMConstructHelper<T>& another) GM_NOEXCEPT
	{
		GM_SWAP(m_ref, another.m_ref);
	}

private:
	T* m_ref;
};

#define GM_DECLARE_PRIVATE_AND_BASE(className, base)										\
	public:																					\
		typedef base Base;																	\
		typedef className##Private Data;													\
	private:																				\
		gm::GMConstructHelper<Data> m_data;													\
	private:																				\
		void noop() {}																		\
	public:																					\
		inline Data* data() const {															\
			(static_cast<const gm::GMObject*>(this));										\
			return const_cast<Data*>(m_data.data()); }										\

#define GM_DECLARE_PRIVATE(className) GM_DECLARE_PRIVATE_AND_BASE(className, gm::GMObject)

/*! \def GM_DECLARE_PRIVATE_NGO
  为一个不是GMObject的对象定义数据段使用的宏。
*/
#define GM_DECLARE_PRIVATE_NGO(className) 													\
	public:																					\
		typedef className##Private Data;													\
	private:																				\
		Data m_data;																		\
		void noop() {}																		\
	public:																					\
		inline Data* data() const { return const_cast<Data*>(&m_data); }

/*! \def GM
  \brief 。

  表示当前GameMachine运行实例。任何时候都推荐用GM来获取GameMachine实例。由于GameMachine实例为单例，因此
  不要尝试创建一个新的GameMachine实例。
*/
#define GM_DECLARE_PRIVATE_FROM_STRUCT(name, anotherStruct) \
	typedef anotherStruct GM_PRIVATE_NAME(name); \
	GM_DECLARE_PRIVATE(name)

// 获取私有成员
#define D(d) auto d = data()
#define D_BASE(d, base) auto d = base::data()
#define D_OF(d, ptr) auto d = (ptr)->data()

// 为一个对象定义private部分
#define GM_PRIVATE_OBJECT(name) class name; GM_ALIGNED_16(struct) name##Private : public gm::GMObjectPrivateBase<name>
#define GM_PRIVATE_OBJECT_FROM(name, extends) class name; GM_ALIGNED_16(struct) name##Private : public extends##Private
#define GM_PRIVATE_NAME(name) name##Private
#define GM_PRIVATE_DESTRUCT(name) ~name##Private()
#define GM_PRIVATE_OBJECT_UNALIGNED(name) class name; struct name##Private
#define GM_PRIVATE_OBJECT_UNALIGNED_FROM(name, extends) class name; struct name##Private : public extends##Private

#define GM_DECLARE_GETTER_ACCESSOR(name, memberName, accessor, callback) \
	accessor: \
	inline auto& get##name() GM_NOEXCEPT { D(d); callback; return d-> memberName; } \
	inline const auto& get##name() const GM_NOEXCEPT { D(d); const_cast<std::remove_const_t<std::remove_pointer_t<decltype(this)>>*>(this)->callback; return d-> memberName; }

#define GM_DECLARE_SETTER_ACCESSOR(name, memberName, accessor, callback) \
	accessor: \
	template <typename T> void set##name(const T& arg) { D(d); d-> memberName = arg; callback; }

#define GM_DECLARE_GETTER(name, memberName) GM_DECLARE_GETTER_ACCESSOR(name, memberName, public, noop()) 
#define GM_DECLARE_SETTER(name, memberName) GM_DECLARE_SETTER_ACCESSOR(name, memberName, public, noop()) 
#define GM_DECLARE_GETTER_WITH_CALLBACK(name, memberName, cb) GM_DECLARE_GETTER_ACCESSOR(name, memberName, public, cb)
#define GM_DECLARE_SETTER_WITH_CALLBACK(name, memberName, cb) GM_DECLARE_SETTER_ACCESSOR(name, memberName, public, cb)

#define GM_DECLARE_PROPERTY(name, memberName) \
	GM_DECLARE_GETTER_ACCESSOR(name, memberName, public, noop()) \
	GM_DECLARE_SETTER_ACCESSOR(name, memberName, public, noop())

#define GM_DECLARE_PROPERTY_WITH_CALLBACK(name, memberName, getterCb, setterCb) \
	GM_DECLARE_GETTER_ACCESSOR(name, memberName, public, getterCb) \
	GM_DECLARE_SETTER_ACCESSOR(name, memberName, public, setterCb)

#define GM_DECLARE_PROTECTED_PROPERTY(name, memberName) \
	GM_DECLARE_GETTER_ACCESSOR(name, memberName, protected, noop()) \
	GM_DECLARE_SETTER_ACCESSOR(name, memberName, protected, noop())

#define GM_DISABLE_COPY(clsName) public: clsName(const clsName&) = delete; clsName(clsName&&) GM_NOEXCEPT = delete;
#define GM_DISABLE_ASSIGN(clsName) public: clsName& operator =(const clsName&) = delete; clsName& operator =(clsName&&) GM_NOEXCEPT = delete;

#define GM_ALLOW_COPY_MOVE(clsName) \
	GM_ALLOW_COPY(clsName) \
	GM_ALLOW_MOVE(clsName)

#define GM_ALLOW_COPY(clsName) \
	public: \
		clsName(const clsName& o) { copyData(o); } \
		clsName& operator=(const clsName& o) { D(d); copyData(o); return *this; } \
		void copyData(const clsName& another) { \
		D(d); D_OF(d_another, &another); *d = *d_another; \
			(static_cast<Base*>(this))->copyData(static_cast<const Base&>(another)); }

#define GM_ALLOW_MOVE(clsName) \
	public: \
	clsName(clsName&& o) GM_NOEXCEPT { swapData(std::move(o)); } \
	clsName& operator=(clsName&& o) GM_NOEXCEPT { D(d); swapData(std::move(o)); return *this; } \
	void swapData(clsName&& another) GM_NOEXCEPT { m_data.swap(another.m_data); \
		(static_cast<Base*>(this))->swapData(std::move(static_cast<Base&>(another))); }

#define GM_FRIEND_CLASS(clsName) \
	friend class clsName; \
	friend struct GM_PRIVATE_NAME(clsName);

#define friend_methods(clsName) private

class GMObject;

enum class GMMetaMemberType
{
	Invalid,
	Int,
	Float,
	Vector2,
	Vector3,
	Vector4,
	Matrix4x4,
	String,
	Boolean,
	Object,
	Pointer,
	Function,
};

struct GMObjectMember
{
	GMMetaMemberType type;
	size_t size;
	void* ptr;
};

using GMMeta = HashMap<GMString, GMObjectMember, GMStringHashFunctor>;

template <typename T>
struct GMMetaMemberTypeGetter
{
	enum { Type = (GMint32) GMMetaMemberType::Invalid };
};

template <>
struct GMMetaMemberTypeGetter<GMint32>
{
	enum { Type = (GMint32) GMMetaMemberType::Int };
};

template <>
struct GMMetaMemberTypeGetter<GMfloat>
{
	enum { Type = (GMint32) GMMetaMemberType::Float };
};

template <>
struct GMMetaMemberTypeGetter<GMVec2>
{
	enum { Type = (GMint32) GMMetaMemberType::Vector2 };
};

template <>
struct GMMetaMemberTypeGetter<GMVec3>
{
	enum { Type = (GMint32) GMMetaMemberType::Vector3 };
};

template <>
struct GMMetaMemberTypeGetter<GMVec4>
{
	enum { Type = (GMint32) GMMetaMemberType::Vector4 };
};

template <>
struct GMMetaMemberTypeGetter<GMMat4>
{
	enum { Type = (GMint32) GMMetaMemberType::Matrix4x4 };
};

template <>
struct GMMetaMemberTypeGetter<GMString>
{
	enum { Type = (GMint32) GMMetaMemberType::String };
};

template <>
struct GMMetaMemberTypeGetter<bool>
{
	enum { Type = (GMint32) GMMetaMemberType::Boolean };
};

template <>
struct GMMetaMemberTypeGetter<GMObject*>
{
	enum { Type = (GMint32) GMMetaMemberType::Object };
};

template <typename T>
struct GMMetaMemberTypeGetter<T*>
{
	enum { Type = (GMint32)GMMetaMemberType::Pointer };
};

// 信号目标，表示一个GMObject被多少信号连接
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

#define GM_SIGNAL(host, signal) host::sig_##signal()
#define GM_DECLARE_SIGNAL(signal) public: inline static const gm::GMString& sig_##signal() { static gm::GMString s_sig(#signal); return s_sig; }

#define GM_META(memberName) \
{ \
	GM_STATIC_ASSERT(static_cast<gm::GMMetaMemberType>( gm::GMMetaMemberTypeGetter<decltype(data()-> memberName)>::Type ) != gm::GMMetaMemberType::Invalid, "Invalid Meta type"); \
	gm::GMObject::data()->meta[#memberName] = { static_cast<gm::GMMetaMemberType>( gm::GMMetaMemberTypeGetter<decltype(data()-> memberName)>::Type ), sizeof(data()-> memberName), &data()->memberName }; \
}

#define GM_META_WITH_TYPE(memberName, type) \
	gm::GMObject::data()->meta[#memberName] = { type, sizeof(data()-> memberName), &data()->memberName };

#define GM_META_FUNCTION(memberName) \
	gm::GMObject::data()->meta[#memberName] = { gm::GMMetaMemberType::Function, 0, (void*)&data()->memberName };

#define GM_END_META_MAP \
	return true; }

// 所有GM对象的基类，如果可以用SSE指令，那么它是16字节对齐的
GM_PRIVATE_OBJECT(GMObject)
{
	bool metaRegistered = false;
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
使用GM_ALLOW_COPY_MOVE宏来允许对象之间相互拷贝，它会生成一套左值和右值拷贝、赋值函数。<BR>
另外，GMObject的数据将会在GMObject构造时被新建，在GMObject析构时被释放。
*/
class GM_EXPORT GMObject : public IVirtualFunctionObject
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
	const GMMeta* meta() const;

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
	  但是，GMObject的子类可以使用GM_ALLOW_COPY_MOVE宏，允许子类调用其copyData方法，依次拷贝私有数据。
	  \param another 拷贝私有数据的目标对象，将目标对象的私有数据拷贝到此对象。
	*/
	void copyData(const GMObject& another) {}

	//! 交换GMObject私有数据
	/*!
	GMObject允许交换其私有数据。<BR>
	\param another 交换私有数据的目标对象，将目标对象的私有数据交换到此对象。
	*/
	void swapData(GMObject&& another) GM_NOEXCEPT
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
#define GM_INTERFACE(name) struct GM_EXPORT name : public gm::IVirtualFunctionObject
#define GM_INTERFACE_FROM(name, base) struct GM_EXPORT name : public base 

// 缓存类，用于存储缓存数据
struct GM_EXPORT GMBuffer
{
	GMBuffer() = default;
	~GMBuffer();

	GMBuffer(const GMBuffer& rhs);
	GMBuffer(GMBuffer&& rhs) GM_NOEXCEPT;
	GMBuffer& operator =(GMBuffer&& rhs) GM_NOEXCEPT;
	GMBuffer& operator =(const GMBuffer& rhs);

public:
	void convertToStringBuffer();
	void convertToStringBufferW();
	void swap(GMBuffer& rhs);

public:
	GMbyte* buffer = nullptr;
	GMsize_t size = 0;
	bool needRelease = false; // 表示是否需要手动释放
};

END_NS
#endif
