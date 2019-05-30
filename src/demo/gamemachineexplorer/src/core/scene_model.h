#ifndef __CORE_SCENE_MODEL_H__
#define __CORE_SCENE_MODEL_H__
#include <gmecommon.h>
#include <QtCore>
#include <type_traits>

namespace core
{
	enum ChangedProperty
	{
		Property_ViewCamera
	};

	struct GlobalProperties
	{
		struct ViewCameraProperties
		{
			int ortho : 1;
			float posX;
			float posY;
			float posZ;
			float lookAtX;
			float lookAtY;
			float lookAtZ;
		} viewCamera;
	};

	static_assert(std::is_pod<GlobalProperties>::value, "GlobalProperties must be POD");

	class SceneModel : public QObject
	{
		Q_OBJECT

	public:
		SceneModel(QObject* parent = nullptr);

	public:
		GlobalProperties& getProperties();
		void notifyPropertyChanged(ChangedProperty);

	private:
		void initProperties();
		void initViewCamera();

	signals:
		void propertyChanged(ChangedProperty);

	private:
		GlobalProperties m_propertyTree = { 0 };
	};
}

#endif