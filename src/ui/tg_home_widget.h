/* ------------------------------------------------------------------
//  File      : tg_home_widget.h
//  Created   : 2026-04-22
//  Purpose   : Custom painted launcher (Apple/Space)
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_HOME_WIDGET_H_
#define _TYPEGAME_TG_HOME_WIDGET_H_

#include <QPixmap>
#include <QWidget>

class TgHomeWidget : public QWidget
{
	Q_OBJECT
public:
	explicit TgHomeWidget(QWidget *pParent = nullptr);
	TgHomeWidget(const TgHomeWidget &) = delete;
	TgHomeWidget &operator=(const TgHomeWidget &) = delete;

signals:
	void appleRequested();
	void spaceRequested();

protected:
	void paintEvent(QPaintEvent *pEvent) override;
	void resizeEvent(QResizeEvent *pEvent) override;
	void mouseMoveEvent(QMouseEvent *pEvent) override;
	void leaveEvent(QEvent *pEvent) override;
	void mouseReleaseEvent(QMouseEvent *pEvent) override;

private:
	enum class HoverTarget
	{
		None,
		Apple,
		Space,
	};

	void loadPixmaps();
	void updateLayout();
	HoverTarget hitTest(const QPoint &pt) const;

	QPixmap m_applePm;
	QPixmap m_spacePm;

	QRect m_appleTile;
	QRect m_spaceTile;
	QRect m_appleImageRect;
	QRect m_spaceImageRect;
	QRect m_appleTitleRect;
	QRect m_spaceTitleRect;

	HoverTarget m_hover = HoverTarget::None;
};

#endif

