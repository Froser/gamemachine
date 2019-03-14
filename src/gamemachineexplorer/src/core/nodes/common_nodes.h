#ifndef __CORE_COMMON_NODES_H__
#define __CORE_COMMON_NODES_H__
#include <gmecommon.h>
#include <core/render_tree.h>

namespace core
{
	class LogoNode : public RenderNode
	{
	public:
		virtual EventResult onMouseMove(SelectedAssets& selectedAssets, const RenderMouseDetails& details) override;

	protected:
		virtual void initAsset(const RenderContext& ctx) override;

	private:
		GMSceneAsset createLogo(const RenderContext& ctx);
	};

	//! 表示一个平面的绘制
	class PlaneNode : public RenderNode
	{
	public:
		virtual EventResult onMouseMove(SelectedAssets& selectedAssets, const RenderMouseDetails& details) override;

	protected:
		virtual void initAsset(const RenderContext& ctx) override;
	};
}

#endif