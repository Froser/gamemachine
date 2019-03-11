#ifndef __HANDLER_H__
#define __HANDLER_H__
#include <gmecommon.h>
#include <gamemachine.h>

namespace core
{
	class Handler : public IGameHandler, public IShaderLoadCallback
	{
	public:
		virtual void init(const IRenderContext* context);
		virtual void start();
		virtual void event(GameMachineHandlerEvent evt);
		virtual void onLoadShaders(const IRenderContext* context) override;

	public:
		const IRenderContext* getContext()
		{
			return m_context;
		}

	private:
		const IRenderContext* m_context = nullptr;
	};

}

#endif