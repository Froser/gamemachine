#ifndef __CORE_HANDLER_CONTROL_H__
#define __CORE_HANDLER_CONTROL_H__
#include <gmecommon.h>
#include <QtCore>
#include <gmassets.h>
#include <gamemachine.h>

namespace core
{
	struct Asset
	{
		GMSceneAsset asset;
		GMGameObject* object = nullptr;
	};

	class Handler;
	class SceneControl : public QObject
	{
		Q_OBJECT

		// 公共资源
		struct Assets
		{
			Asset logo;
			Asset plain;
		};

		struct Light
		{
			GMLightIndex defaultLightIndex = -1;
		};

	public:
		SceneControl(Handler* handler, QObject* parent = nullptr);

	public slots:
		void onSceneModelCreated(SceneModel* model);

	// 对场景的操作
	public:
		void setViewCamera(const GMCamera& camera);
		void setDefaultColor(const GMVec4& color);
		void setDefaultLight(const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity);
		void clearRenderList();
		void renderLogo();
		void renderPlain();

	signals:
		void renderUpdate();

	protected:
		virtual void resetModel(SceneModel*);
		virtual GMAsset createLogo();

	private:
		Handler* m_handler = nullptr;
		SceneModel* m_model = nullptr;
		Assets m_assets;
		Light m_lights;

		// 场景相关组件
		GMCamera m_sceneViewCamera;
	};
}

#endif