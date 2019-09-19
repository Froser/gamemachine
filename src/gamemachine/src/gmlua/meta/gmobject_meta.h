#ifndef __GM_LUA_GMOBJECT_META_H__
#define __GM_LUA_GMOBJECT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

#define GM_LUA_PRIVATE_CLASS_FROM(clsName, baseName) \
struct GM_PRIVATE_NAME(clsName) : public baseName \
{ \
	GM_PRIVATE_NAME(clsName)() { } \
	GM_PRIVATE_NAME(clsName)& operator=(const baseName& rhs) \
	{ \
		memcpy_s(this, sizeof(GM_PRIVATE_NAME(clsName)), &rhs, sizeof(baseName)); \
		return *this; \
	} \
};

namespace luaapi
{
	GM_PRIVATE_CLASS(GMObjectProxy);
	class GMObjectProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMObjectProxy)

	public: 
		GMObjectProxy(GMLuaCoreState* l, GMObject* handler = nullptr);
		~GMObjectProxy();

		GMLuaCoreState* getLuaCoreState() const;
		void setObjectName(const GMString& name);
		GMObject* get() const;
		GMObject* operator->() const;
		void set(GMObject* handler);
		void setAutoRelease(bool);

	public:
		operator bool() const
		{
			return !!get();
		}

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_CLASS(GMAnyProxy);
	class GMAnyProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMAnyProxy)

	public:
		GMAnyProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		~GMAnyProxy();

		void setObjectName(const GMString& name);
		GMLuaCoreState* getLuaCoreState() const;
		IDestroyObject* get() const;
		IDestroyObject* operator->() const;
		void set(IDestroyObject* handler);
		void setAutoRelease(bool);

	public:
		operator bool() const
		{
			D(d);
			return !!get();
		}

	protected:
		virtual bool registerMeta() override;
	};

#define LUA_PROXY(className) className* __handler = nullptr; GMString __name = #className;

	GM_PRIVATE_OBJECT_UNALIGNED(GMObjectProxy)
	{
		LUA_PROXY(GMObject);
		GMLuaCoreState* l = nullptr;
		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(connect);
		GM_LUA_PROXY_FUNC(emitSignal);
	};

	GM_PRIVATE_OBJECT_UNALIGNED(GMAnyProxy)
	{
		LUA_PROXY(IDestroyObject);

		GM_LUA_PROXY_FUNC(__gc);
		GMLuaCoreState* l = nullptr;
	};

#undef LUA_PROXY
}

END_NS
#endif