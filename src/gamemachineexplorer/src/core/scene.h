#ifndef __CORE_SCENE_H__
#define __CORE_SCENE_H__
#include <gmecommon.h>
#include <QtCore>

namespace core
{
	class Scene : public QObject
	{
		Q_OBJECT

		using QObject::QObject;

	public:
		void setControl(HandlerControl* control);
		void setWidget(GameMachineWidget* widget);
		void setModel(SceneModel* model);

	public:
		HandlerControl* control();
		GameMachineWidget* widget();
		SceneModel* model();

	private:
		HandlerControl* m_control = nullptr;
		SceneModel* m_model = nullptr;
		GameMachineWidget* m_widget = nullptr;
	};
}

#endif