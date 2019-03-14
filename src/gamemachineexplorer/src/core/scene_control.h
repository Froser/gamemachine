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

	inline bool operator ==(const Asset& lhs, const Asset& rhs)
	{
		return (lhs.asset == rhs.asset) && (lhs.shape == rhs.shape) && (lhs.object == rhs.object);
	}

	class Handler;
	class SceneControl : public QObject
	{
		Q_OBJECT

		// 公共资源
		enum AssetType
		{
			AT_Begin,
			AT_Logo = AT_Begin,
			AT_Plane,
			AT_End,
		};

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
		void setViewCamera(const GMCamera& camera);
		void setDefaultColor(const GMVec4& color);
		void setDefaultLight(const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity);
		const GMCamera& currentCamera();

		void clearRenderList();
		void renderLogo();
		void renderPlain();

	signals:
		void renderUpdate();

	protected:
		virtual void resetModel(SceneModel*);
		virtual GMAsset createLogo();
		virtual Asset hitTest(int x, int y);
		virtual Asset findAsset(GMPhysicsObject*);

	private:
		// 基本元素
		Handler* m_handler = nullptr;
		SceneModel* m_model = nullptr;
		std::array<Asset, AT_End> m_assets;
		Light m_defaultLight;

		// UIL
		QList<Asset> m_selectedAssets;
		bool m_mouseDown = false;
		QPoint m_mouseDownPos;

		// 场景相关组件
		GMCamera m_sceneViewCamera;
	};
}

#endif