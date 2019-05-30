#ifndef __CORE_HANDLER_H__
#define __CORE_HANDLER_H__
#include <gmecommon.h>
#include <gamemachine.h>

namespace core
{
	class Handler : public IGameHandler, public IShaderLoadCallback
	{
	public:
		~Handler();

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

		GMGameWorld* getWorld()
		{
			return m_world;
		}

		GMDiscreteDynamicsWorld* getPhysicsWorld()
		{
			return m_physicsWorld;
		}

	private:
		const IRenderContext* m_context = nullptr;
		GMGameWorld* m_world = nullptr;
		GMDiscreteDynamicsWorld* m_physicsWorld = nullptr;
	};

}

#endif