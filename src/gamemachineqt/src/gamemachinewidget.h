#ifndef __GAMEMACHINEWIDGET_H__
#define __GAMEMACHINEWIDGET_H__
#include <QWidget>
#include <gamemachine.h>
#include "forwards.h"

namespace gm
{
	GM_PRIVATE_CLASS(GameMachineWidget);
	class GM_QT_EXPORT GameMachineWidget : public QWidget
	{
		Q_OBJECT
		GM_DECLARE_PRIVATE(GameMachineWidget)

	public:
		explicit GameMachineWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

	public:
		~GameMachineWidget();

	public:
		void setGameMachine(const GMGameMachineDesc& desc, float renderWidth, float renderHeight, IGameHandler* handler);

	protected:
		void setRenderContext(const IRenderContext* context);

	protected:
		virtual bool event(QEvent* e) override;
		virtual QPaintEngine* paintEngine() const override;
		virtual void paintEvent(QPaintEvent *event) override;
	};
}

#endif