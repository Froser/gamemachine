#ifndef __CORE_RENDER_TREE_H__
#define __CORE_RENDER_TREE_H__
#include <gmecommon.h>
#include <QtCore>
#include <gamemachine.h>

namespace core
{
	struct Asset
	{
		GMSceneAsset asset;
		GMPhysicsShapeAsset shape;
		GMGameObject* object = nullptr;
	};

	typedef QList<RenderTree*> RenderTrees;
	typedef QList<RenderNode*> SelectedNodes;

	inline bool operator ==(const Asset& lhs, const Asset& rhs)
	{
		return (lhs.asset == rhs.asset) && (lhs.shape == rhs.shape) && (lhs.object == rhs.object);
	}

	struct RenderMouseDetails
	{
		bool mouseDown;
		int position[2];
		int lastPosition[2];
	};

	struct IContextCallback
	{
		virtual SelectedNodes selectedNotes() = 0;
	};

	struct RenderContext
	{
		Handler* handler;
		SceneControl* control;
		IContextCallback* callback;
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
		virtual EventResult onMousePress(const RenderContext& ctx, const RenderMouseDetails& details);
		virtual EventResult onMouseRelease(const RenderContext& ctx, const RenderMouseDetails& details);
		virtual EventResult onMouseMove(const RenderContext& ctx, const RenderMouseDetails& details);

	protected:
		virtual bool isAssetReady() const;
		virtual void renderAsset(const RenderContext&);
		virtual void initAsset(const RenderContext& ctx);

	protected:
		Asset m_asset;

	private:
		QList<RenderNode*> m_nodes;
	};

	class RenderTree : public QObject, public IContextCallback
	{
		Q_OBJECT

	public:
		RenderTree(SceneControl*);
		~RenderTree();

	public:
		void setRoot(RenderNode* root);

	public:
		virtual void render(bool cleanBuffer);
		virtual RenderNode* hitTest(int x, int y);
		virtual void onMousePress(const RenderMouseDetails& details);
		virtual void onMouseRelease(const RenderMouseDetails& details);
		virtual void onMouseMove(const RenderMouseDetails& details);

		// IContextCallback
	public:
		virtual SelectedNodes selectedNotes() override;

	private:
		RenderNode* m_root = nullptr;
		SceneControl* m_control = nullptr;
		RenderContext m_ctx;
	};
}

#endif