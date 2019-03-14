#ifndef __CORE_HANDLER_CONTROL_H__
#define __CORE_HANDLER_CONTROL_H__
#include <gmecommon.h>
#include <QtCore>
#include <QtWidgets>
#include <gmassets.h>
#include <gamemachine.h>

namespace core
{
	struct Asset
	{
		GMSceneAsset asset;
		GMPhysicsShapeAsset shape;
		GMGameObject* object = nullptr;
	};

	typedef QList<Asset> SelectedAssets;
	typedef QList<RenderTree*> RenderTrees;

	inline bool operator ==(const Asset& lhs, const Asset& rhs)
	{
		return (lhs.asset == rhs.asset) && (lhs.shape == rhs.shape) && (lhs.object == rhs.object);
	}

	class Handler;
	class SceneControl : public QObject
	{
		Q_OBJECT

		struct Light
		{
			GMLightIndex defaultLightIndex = -1;
		};

	public:
		SceneControl(Handler* handler, QObject* parent = nullptr);

	public slots:
		void onSceneModelCreated(SceneModel* model);
		void onWidgetMousePress(shell::View*, QMouseEvent*);
		void onWidgetMouseRelease(shell::View*, QMouseEvent*);
		void onWidgetMouseMove(shell::View*, QMouseEvent*);

	// 对场景的操作
	public:
		Handler* getHandler();
		void setViewCamera(const GMCamera& camera);
		void setDefaultColor(const GMVec4& color);
		void setDefaultLight(const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity);
		const GMCamera& currentCamera();

		void clearRenderList();

	signals:
		void renderUpdate();

	protected:
		virtual void resetModel(SceneModel*);
		virtual GMAsset createLogo();
		virtual RenderNode* hitTest(int x, int y);

	private:
		// 基本元素
		Handler* m_handler = nullptr;
		SceneModel* m_model = nullptr;
		RenderTrees m_renderTrees;
		Light m_defaultLight;

		// UIL
		SelectedAssets m_selectedAssets;
		bool m_mouseDown = false;
		QPoint m_mouseDownPos;

		// 场景相关组件
		RenderTree* m_currentRenderTree = nullptr;
		GMCamera m_sceneViewCamera;
	};
}

#endif