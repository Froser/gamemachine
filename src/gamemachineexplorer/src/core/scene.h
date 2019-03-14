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
		void newFile();

	public:
		void setControl(SceneControl* control);
		SceneControl* control();

		void setWidget(shell::View* widget);
		shell::View* widget();

		void setModel(SceneModel* model);
		SceneModel* model();

	private slots:
		void onRenderUpdate();

	signals:
		void fileCreated(SceneModel*);

	private:
		void connectWidgetAndControl();

	private:
		SceneControl* m_control = nullptr;
		SceneModel* m_model = nullptr;
		shell::View* m_widget = nullptr;
	};
}

#endif