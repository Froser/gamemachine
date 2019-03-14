#include "stdafx.h"
#include "render_tree.h"
#include "handler.h"
#include <gmdiscretedynamicsworld.h>
#include "scene_control.h"

#define SAFE_DELETE(p) { delete (p); p = nullptr;  }

namespace
{
	core::RenderNode* find(const QList<core::RenderNode*>& container, GMGameObject* obj)
	{
		foreach(auto node, container)
		{
			if (node->asset().object == obj)
				return node;
		}
		return nullptr;
	}
}

namespace core
{
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
	}

	bool RenderNode::hitTest(const RenderContext& ctx)
	{
		return true;
	}

	EventResult RenderNode::onMousePress(const RenderContext& ctx, const RenderMouseDetails& details)
	{
		ctx.tree->clearSelect();
		ctx.tree->select(this);
		return ER_Continue;
	}

	EventResult RenderNode::onMouseRelease(const RenderContext& ctx, const RenderMouseDetails& details)
	{
		return ER_Continue;
	}

	EventResult RenderNode::onMouseMove(const RenderContext& ctx, const RenderMouseDetails& details)
	{
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
		foreach(auto node, m_nodes)
		{
			SAFE_DELETE(node);
		}
	}

	void RenderTree::appendNode(RenderNode* node)
	{
		m_nodes << node;
	}

	void RenderTree::reset()
	{
		clearSelect();
	}

	void RenderTree::render(bool cleanBuffer)
	{
		if (cleanBuffer)
			m_control->clearRenderList();

		foreach(auto node, m_nodes)
		{
			node->render(m_ctx);
		}
	}

	RenderNode* RenderTree::hitTest(int x, int y)
	{
		const GMCamera& camera = m_control->currentCamera();
		GMVec3 rayFrom = camera.getLookAt().position;
		GMVec3 rayTo = camera.getRayToWorld(m_ctx.handler->getContext()->getWindow()->getRenderRect(), x, y);
		GMPhysicsRayTestResult rayTestResult = m_ctx.handler->getPhysicsWorld()->rayTest(rayFrom, rayTo);
		if (rayTestResult.hit)
		{
			RenderNode* candidate = find(m_nodes, rayTestResult.hitObject->getGameObject());
			if (candidate && candidate->hitTest(m_ctx))
				return candidate;
		}
		return nullptr;
	}

	void RenderTree::onMousePress(const RenderMouseDetails& details, RenderNode* hitTestResult)
	{
		foreach (auto node, m_nodes)
		{
			if (hitTestResult == node && node->onMousePress(m_ctx, details) == ER_OK)
				break;
		}
	}

	void RenderTree::onMouseRelease(const RenderMouseDetails& details)
	{
		foreach(auto node, m_nodes)
		{
			if (!m_selection.contains(node))
				continue;

			if (node->onMouseRelease(m_ctx, details) == ER_OK)
				break;
		}
	}

	void RenderTree::onMouseMove(const RenderMouseDetails& details)
	{
		foreach(auto node, m_nodes)
		{
			if (!m_selection.contains(node))
				continue;

			if (node->onMouseMove(m_ctx, details) == ER_OK)
				break;
		}
	}

	void RenderTree::clearSelect()
	{
		m_selection.clear();
	}

	void RenderTree::select(RenderNode* node)
	{
		if (!m_selection.contains(node))
			m_selection << node;
	}

	SelectedNodes RenderTree::selectedNotes()
	{
		return m_selection;
	}

}