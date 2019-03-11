#ifndef __GAMEMACHINEWIDGET_H__
#define __GAMEMACHINEWIDGET_H__
#include <gmecommon.h>
#include <QWidget>

namespace shell
{
	class GameMachineWidget : public QWidget
	{
		Q_OBJECT

	private:
		explicit GameMachineWidget(QWidget* parent = nullptr);

	public:
		static GameMachineWidget* createGameMachineWidget(const GMGameMachineDesc& desc, IGameHandler* handler, QWidget* parent = nullptr);

	protected:
		void setRenderContext(const IRenderContext* context);

	protected:
		virtual QPaintEngine* paintEngine() const override;
		virtual void paintEvent(QPaintEvent *event) override;

	private:
		const IRenderContext* m_context = nullptr;
	};
}

#endif