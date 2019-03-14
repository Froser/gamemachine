#ifndef __CORE_HANDLER_CONTROL_H__
#define __CORE_HANDLER_CONTROL_H__
#include <gmecommon.h>
#include <QtCore>
#include <QtWidgets>
#include <gmassets.h>
#include <gamemachine.h>
#include "render_tree.h"

namespace core
{
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

	public:
		virtual void init();

	// 对场景的操作
	public:
		Handler* getHandler();
		const GMCamera& viewCamera();
		void setViewCamera(const GMCamera& camera);
		void setCamera(const GMCamera& camera);
		void setDefaultColor(const GMVec4& color);
		void setDefaultLight(const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity);
		const GMCamera& currentCamera();
		void clearRenderList();
		void update();

	signals:
		void renderUpdate();

	protected:
		virtual void resetModel(SceneModel*);
		virtual RenderNode* hitTest(int x, int y);

	protected:
		void render();
		void setCurrentRenderTree(RenderTree*);

	private:
		RenderMouseDetails mouseDetails(const QMouseEvent* e);
		RenderTree* createRenderTree_Splash();
		RenderTree* createRenderTree_Scene();

	private:
		// 基本元素
		Handler* m_handler = nullptr;
		SceneModel* m_model = nullptr;
		Light m_defaultLight;
		RenderTree* m_splashTree = nullptr;
		RenderTree* m_sceneTree = nullptr;

		// UIL
		bool m_mouseDown = false;
		QPoint m_mouseDownPos;

		// 场景相关组件
		RenderTree* m_currentRenderTree = nullptr;
		GMCamera m_sceneViewCamera;
	};
}

#endif