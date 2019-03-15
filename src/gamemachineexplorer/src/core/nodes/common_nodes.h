#ifndef __CORE_COMMON_NODES_H__
#define __CORE_COMMON_NODES_H__
#include <gmecommon.h>
#include <core/render_tree.h>

namespace core
{
	class SplashRenderTree : public RenderTree
	{
		using RenderTree::RenderTree;

	public:
		virtual void onRenderTreeSet() override;
	};

	class SplashNode : public RenderNode
	{
	public:
		virtual EventResult onMouseMove(const RenderContext& ctx, const RenderMouseDetails& details) override;

	protected:
		virtual void initAsset(const RenderContext& ctx) override;
		virtual bool hitTest(const RenderContext& ctx) override;

	private:
		GMSceneAsset createSplash(const RenderContext& ctx);
	};

	//! 表示一个平面的绘制
	class SceneRenderTree : public RenderTree
	{
		using RenderTree::RenderTree;

	public:
		virtual void onRenderTreeSet() override;
	};

	class PlaneNode : public RenderNode
	{
	public:
		virtual EventResult onMouseMove(const RenderContext& ctx, const RenderMouseDetails& details) override;

	protected:
		virtual void initAsset(const RenderContext& ctx) override;
	};
}

#endif