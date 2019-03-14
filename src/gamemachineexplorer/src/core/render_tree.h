#ifndef __CORE_RENDER_TREE_H__
#define __CORE_RENDER_TREE_H__
#include <gmecommon.h>
#include <QtCore>
#include "scene_control.h"

namespace core
{
	struct RenderMouseDetails
	{
		bool mouseDown;
		int position[2];
		int lastPosition[2];
	};

	struct RenderContext
	{
		Handler* handler;
		SceneControl* control;
	};

	enum EventResult
	{
		ER_OK,
		ER_Continue,
	};

	class RenderNode
	{
	public:
		~RenderNode();

	public:
		Asset asset();
		void setAsset(const Asset& asset);

	public:
		virtual void render(const RenderContext&);
		virtual RenderNode* hitTest(const RenderContext&, int x, int y);
		virtual EventResult onMouseMove(SelectedAssets& selectedAssets, const RenderMouseDetails& details);

	protected:
		virtual bool isAssetReady() const;
		virtual void renderAsset(const RenderContext&);
		virtual void initAsset(const RenderContext& ctx);

	protected:
		Asset m_asset;

	private:
		QList<RenderNode*> m_nodes;
	};

	class RenderTree : public QObject
	{
		Q_OBJECT

	public:
		RenderTree(SceneControl*);
		~RenderTree();

	public:
		virtual void render(bool cleanBuffer);
		virtual RenderNode* hitTest(int x, int y);
		virtual void onMouseMove(SelectedAssets& selectedAssets, const RenderMouseDetails& details);

	private:
		RenderNode* m_root = nullptr;
		SceneControl* m_control = nullptr;
		RenderContext m_ctx;
	};
}

#endif