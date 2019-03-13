#include "stdafx.h"
#include "scene_model.h"

namespace core
{
	SceneModel::SceneModel(QObject* parent /*= nullptr*/)
		: QObject(parent)
	{
		initProperties();
	}

	GlobalProperties& SceneModel::getProperties()
	{
		return m_propertyTree;
	}

	void SceneModel::notifyPropertyChanged(ChangedProperty p)
	{
		emit propertyChanged(p);
	}

	void SceneModel::initProperties()
	{
		initViewCamera();
	}

	void SceneModel::initViewCamera()
	{
		m_propertyTree.viewCamera.posX = m_propertyTree.viewCamera.posY = m_propertyTree.viewCamera.posZ = 40;
		m_propertyTree.viewCamera.lookAtX = m_propertyTree.viewCamera.lookAtY = m_propertyTree.viewCamera.lookAtZ = 0;
		notifyPropertyChanged(Property_ViewCamera);
	}

}
