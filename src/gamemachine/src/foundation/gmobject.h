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

// 对象存储
GM_ALIGNED_16(struct) GMObjectPrivateAlignedBase
{
	GM_DECLARE_ALIGNED_ALLOCATOR();
	virtual ~GMObjectPrivateAlignedBase() {}
};

#define GM_DECLARE_PRIVATE(className)														\
	public:																					\
		typedef className##Private Data;													\
	protected:																				\
		gm::GMOwnedPtr<Data> _gm_data;														\
		inline Data* data() const {															\
			return const_cast<Data*>(_gm_data.get()); }

#define GM_DECLARE_BASE(base) typedef base Base;

/*! \def GM
  \brief 。

  表示当前GameMachine运行实例。任何时候都推荐用GM来获取GameMachine实例。由于GameMachine实例为单例，因此
  不要尝试创建一个新的GameMachine实例。
*/

// 获取私有成员
#define D(d) auto d = data(); GM_ASSERT(d)
#define D_BASE(d, base) auto d = base::data(); GM_ASSERT(d)
#define D_OF(d, ptr) auto d = (ptr)->data(); GM_ASSERT(d)

// 为一个对象定义private部分
#define GM_PRIVATE_NAME(name) name##Private
#define GM_PRIVATE_CLASS(name) struct GM_PRIVATE_NAME(name)
#define GM_PRIVATE_DESTRUCTOR(name) ~GM_PRIVATE_NAME(name)()

#define GM_PRIVATE_OBJECT_ALIGNED(name) GM_ALIGNED_16(struct) GM_PRIVATE_NAME(name) : public gm::GMObjectPrivateAlignedBase
#define GM_PRIVATE_OBJECT_ALIGNED_FROM(name, extends) GM_ALIGNED_16(struct) GM_PRIVATE_NAME(name) : public GM_PRIVATE_NAME(extends)
#define GM_PRIVATE_OBJECT_UNALIGNED(name) struct GM_PRIVATE_NAME(name)
#define GM_PRIVATE_OBJECT_UNALIGNED_FROM(name, extends) struct GM_PRIVATE_NAME(name) : public GM_PRIVATE_NAME(extends)

#define GM_CREATE_DATA() { _gm_data.reset(new Data()); }

//////////////////////////////////////////////////////////////////////////
#define GM_DECLARE_INLINE_GETTER_ACCESSOR(name, memberName, accessor) \
	accessor: \
	auto& get##name() GM_NOEXCEPT { return memberName; } \
	const auto& get##name() const GM_NOEXCEPT { return memberName; }

#define GM_DECLARE_INLINE_SETTER_ACCESSOR(name, memberName, accessor) \
	accessor: \
	template <typename T> void set##name(T&& arg) { memberName = std::forward<T>(arg); }

#define GM_DECLARE_INLINE_GETTER(name, memberName) GM_DECLARE_INLINE_GETTER_ACCESSOR(name, memberName, public) 
#define GM_DECLARE_INLINE_SETTER(name, memberName) GM_DECLARE_INLINE_SETTER_ACCESSOR(name, memberName, public) 

#define GM_DECLARE_INLINE_PROPERTY(name, memberName) \
	GM_DECLARE_INLINE_GETTER_ACCESSOR(name, memberName, public) \
	GM_DECLARE_INLINE_SETTER_ACCESSOR(name, memberName, public)

//////////////////////////////////////////////////////////////////////////
#define GM_DECLARE_EMBEDDED_GETTER_ACCESSOR(name, memberName, accessor) \
	accessor: \
	auto& get##name() GM_NOEXCEPT { D(d); return d-> memberName; } \
	const auto& get##name() const GM_NOEXCEPT { D(d); return d-> memberName; }

#define GM_DECLARE_EMBEDDED_SETTER_ACCESSOR(name, memberName, accessor) \
	accessor: \
	template <typename T> void set##name(T&& arg) { D(d); d-> memberName = std::forward<T>(arg); }

#define GM_DECLARE_EMBEDDED_GETTER(name, memberName) GM_DECLARE_EMBEDDED_GETTER_ACCESSOR(name, memberName, public) 
#define GM_DECLARE_EMBEDDED_SETTER(name, memberName) GM_DECLARE_EMBEDDED_SETTER_ACCESSOR(name, memberName, public) 

#define GM_DECLARE_EMBEDDED_PROPERTY(name, memberName) \
	GM_DECLARE_EMBEDDED_GETTER_ACCESSOR(name, memberName, public) \
	GM_DECLARE_EMBEDDED_SETTER_ACCESSOR(name, memberName, public)

//////////////////////////////////////////////////////////////////////////
#define GM_DECLARE_GETTER_ACCESSOR(type, name, accessor) \
	accessor: \
	type& get##name() GM_NOEXCEPT; \
	const type& get##name() const GM_NOEXCEPT;

#define GM_DECLARE_SETTER_ACCESSOR(type, name, accessor) \
	accessor: \
	void set##name(const type& arg); \
	void set##name(type&& arg) GM_NOEXCEPT; \

#define GM_DEFINE_GETTER(clsName, type, name, memberName) \
	type& clsName::get##name() GM_NOEXCEPT { D(d); return d-> memberName; } \
	const type& clsName::get##name() const GM_NOEXCEPT { D(d); return d-> memberName; }

#define GM_DEFINE_SETTER(clsName, type, name, memberName) \
	void clsName::set##name(const type& arg) { D(d); d-> memberName = arg; } \
	void clsName::set##name(type&& arg) GM_NOEXCEPT { D(d); d->memberName = std::move(arg); }

#define GM_DECLARE_PROPERTY(type, name) \
	GM_DECLARE_GETTER_ACCESSOR(type, name, public) \
	GM_DECLARE_SETTER_ACCESSOR(type, name, public)

#define GM_DEFINE_PROPERTY(clsName, type, name, memberName) \
	GM_DEFINE_GETTER(clsName, type, name, memberName) \
	GM_DEFINE_SETTER(clsName, type, name, memberName)

#define GM_DISABLE_COPY(clsName) public: clsName(const clsName&) = delete; clsName(clsName&&) GM_NOEXCEPT = delete;
#define GM_DISABLE_ASSIGN(clsName) public: clsName& operator =(const clsName&) = delete; clsName& operator =(clsName&&) GM_NOEXCEPT = delete;
#define GM_DISABLE_COPY_ASSIGN(clsName) \
	GM_DISABLE_COPY(clsName) \
	GM_DISABLE_ASSIGN(clsName)

#define GM_FRIEND_CLASS(clsName) \
	friend class clsName; \
	friend struct GM_PRIVATE_NAME(clsName);

#define GM_FRIEND_STRUCT(clsName) \
	friend struct clsName; \
	friend struct GM_PRIVATE_NAME(clsName);

#define GM_DECLARE_POINTER(name) class name; typedef name *name##Ptr;

#define friend_methods(clsName) private

#define GM_COPY(rhs) \
	{ GM_CREATE_DATA(); \
	D(d); \
	D_OF(d_rhs, &rhs); \
	*d = *d_rhs ; }

#define GM_MOVE(rhs) \
	{ using namespace std; swap(_gm_data, rhs._gm_data); }

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

#define GM_SIGNAL(host, sig) host::sig_##sig()
#define GM_DECLARE_SIGNAL(sig) public: inline static const gm::GMString& sig_##sig() { static gm::GMString s_sig(#sig); return s_sig; }

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

GM_PRIVATE_OBJECT_ALIGNED(GMObject)
{
	bool metaRegistered = false;
	GMMeta meta;
	GMSlots objSlots;
	GMThreadId tid = 0;
	GMConnectionTargets connectionTargets;
};
// 所有GM对象的基类，如果可以用SSE指令，那么它是16字节对齐的

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
class GM_EXPORT GMObject : public IDestroyObject
{
	GM_DECLARE_PRIVATE(GMObject)
	GM_DECLARE_BASE(IDestroyObject)
	GM_DISABLE_COPY(GMObject)
	GM_DISABLE_ASSIGN(GMObject)

public:
	//! GMObject构造函数。
	/*!
	  构造一个GMObject实例。
	*/
	GMObject();

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
	  \param sig 绑定的信号。
	  \param callback 信号被触发时执行的回调函数。
	  \sa detachEvent()
	*/
	void connect(GMObject& sender, GMSignal sig, const GMEventCallback& callback);

	//! 从某对象源中解绑一个信号。
	/*!
	  当从某对象解除绑定信号后，对象源的信号被触发后将不会通知此对象。
	  \param sender 对象源。也就是信号的产生方。
	  \param sig 需要解除的信号名。
	*/
	void disconnect(GMObject& sender, GMSignal sig);

	//! 触发一个信号。
	/*!
	  当一个信号被触发后，将会通知所有绑定了此对象此信号的所有对象，调用它们绑定的回调函数。
	  \param sig 需要触发的信号名。
	*/
	void emitSignal(GMSignal sig);

	//! 获取对象创建时的线程ID。
	/*!
	  return 对象创建时的线程ID。
	*/
	GMThreadId getThreadId() GM_NOEXCEPT;

	//! 将一个对象移交到另外一个线程。
	/*!
	  \param tid 将要移交到的线程。
	*/
	void moveToThread(GMThreadId tid) GM_NOEXCEPT;

private:
	void addConnection(GMSignal sig, GMObject& receiver, GMEventCallback callback);
	void removeSignalAndConnection(GMSignal sig, GMObject& receiver);
	void removeSignal(GMSignal sig, GMObject& receiver);
	void releaseConnections();
	void addConnection(GMObject* host, GMSignal sig);
	void removeConnection(GMObject* host, GMSignal sig);

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
#define GM_INTERFACE(name) struct GM_EXPORT name : public gm::IDestroyObject
#define GM_INTERFACE_FROM(name, base) struct GM_EXPORT name : public base

END_NS
#endif
