#ifndef __CORE_HANDLER_CONTROL_H__
#define __CORE_HANDLER_CONTROL_H__
#include <gmecommon.h>
#include <QtCore>
#include <gmassets.h>
#include <gamemachine.h>

namespace core
{
	class Handler;
	class HandlerControl : public QObject
	{
		Q_OBJECT

		struct Assets
		{
			GMSceneAsset logo;
			GMGameObject* logoObj = nullptr;
		};

		struct Light
		{
			GMLightIndex defaultLightIndex = -1;
		};

	public:
		HandlerControl(Handler* handler, QObject* parent = nullptr);

	public:
		void setDefaultCamera(const GMCamera& camera);
		void setDefaultColor(const GMVec4& color);
		void setDefaultLight(const GMVec3& position, const GMVec3& diffuseIntensity, const GMVec3& ambientIntensity);
		void clearRenderList();
		void renderLogo();

	protected:
		virtual GMAsset createLogo();

	private:
		Handler* m_handler = nullptr;
		Assets m_assets;
		Light m_lights;
	};
}

#endif