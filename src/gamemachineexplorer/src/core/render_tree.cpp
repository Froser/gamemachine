#include "stdafx.h"
#include "render_tree.h"
#include "handler.h"
#include <gmdiscretedynamicsworld.h>
#include "scene_control.h"

#define SAFE_DELETE(p) { delete (p); p = nullptr;  }

namespace core
{
	RenderNode::~RenderNode()
	{
		foreach(auto node, m_nodes)
		{
			SAFE_DELETE(node);
		}
	}

	Asset RenderNode::asset()
	{
		return m_asset;
	}

	void RenderNode::setAsset(const Asset& asset)
	{
		m_asset = asset;
	}

	void RenderNode::render(const RenderContext& ctx)
	{
		if (isAssetReady())
		{
			renderAsset(ctx);
		}
		else
		{
			initAsset(ctx);
			if (isAssetReady())
				renderAsset(ctx);
		}

		foreach(auto node, m_nodes)
		{
			if (node)
				node->render(ctx);
		}
	}

	RenderNode* RenderNode::hitTest(const RenderContext& ctx, int x, int y)
	{
		const GMCamera& camera = ctx.control->currentCamera();
		GMVec3 rayFrom = camera.getLookAt().position;
		GMVec3 rayTo = camera.getRayToWorld(ctx.handler->getContext()->getWindow()->getRenderRect(), x, y);
		GMPhysicsRayTestResult rayTestResult = ctx.handler->getPhysicsWorld()->rayTest(rayFrom, rayTo);

		foreach(auto node, m_nodes)
		{
			if (node)
			{
				RenderNode* hitTestNode = node->hitTest(ctx, x, y);
				if (hitTestNode)
					return hitTestNode;
			}
		}
		return nullptr;
	}

	EventResult RenderNode::onMousePress(const RenderContext& ctx, const RenderMouseDetails& details)
	{
		foreach(auto node, m_nodes)
		{
			if (node && node->onMousePress(ctx, details) == ER_OK)
				return ER_OK;
		}
		return ER_Continue;
	}

	EventResult RenderNode::onMouseRelease(const RenderContext& ctx, const RenderMouseDetails& details)
	{
		foreach(auto node, m_nodes)
		{
			if (node && node->onMouseRelease(ctx, details) == ER_OK)
				return ER_OK;
		}
		return ER_Continue;
	}

	EventResult RenderNode::onMouseMove(const RenderContext& ctx, const RenderMouseDetails& details)
	{
		foreach(auto node, m_nodes)
		{
			if (node && node->onMouseMove(ctx, details) == ER_OK)
				return ER_OK;
		}
		return ER_Continue;
	}

	bool RenderNode::isAssetReady() const
	{
		return !m_asset.asset.isEmpty();
	}

	void RenderNode::renderAsset(const RenderContext& ctx)
	{
		ctx.handler->getWorld()->addToRenderList(m_asset.object);
	}

	void RenderNode::initAsset(const RenderContext& ctx)
	{

	}

	RenderTree::RenderTree(SceneControl* control)
		: QObject(control)
		, m_control(control)
	{
		m_ctx = {
			m_control->getHandler(),
			m_control,
			this
		};
	}

	RenderTree::~RenderTree()
	{
		SAFE_DELETE(m_root);
	}

	void RenderTree::setRoot(RenderNode* root)
	{
		if (m_root && m_root != root)
			delete m_root;

		m_root = root;
	}

	void RenderTree::render(bool cleanBuffer)
	{
		if (cleanBuffer)
		{
			m_ctx.handler->getWorld()->clearRenderList();
		}

		if (m_root)
		{
			m_root->render(m_ctx);
		}
	}

	RenderNode* RenderTree::hitTest(int x, int y)
	{
		m_control->clearSelect();

		if (m_root)
		{
			return m_root->hitTest(m_ctx, x, y);
		}
		return nullptr;
	}

	void RenderTree::onMousePress(const RenderMouseDetails& details)
	{
		m_control->clearSelect();

		if (m_root)
		{
			m_root->onMousePress(m_ctx, details);
		}
	}

	void RenderTree::onMouseRelease(const RenderMouseDetails& details)
	{
		if (m_root)
		{
			m_root->onMouseRelease(m_ctx, details);
		}
	}

	void RenderTree::onMouseMove(const RenderMouseDetails& details)
	{
		if (m_root)
		{
			m_root->onMouseMove(m_ctx, details);
		}
	}

	SelectedNodes RenderTree::selectedNotes()
	{
		return m_control->selectedNodes();
	}

}