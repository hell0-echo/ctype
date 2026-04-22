/* ------------------------------------------------------------------
//  File      : tg_skinslider.h
//  Created   : 2026-04-21
//  Purpose   : Custom painted slider (no QSS)
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_SKINSLIDER_H_
#define _TYPEGAME_TG_SKINSLIDER_H_

#include <QPixmap>
#include <QSlider>

class TgSkinSlider : public QSlider
{
	Q_OBJECT
public:
	explicit TgSkinSlider(const QPixmap &groove, const QPixmap &handleTwoStates,
		Qt::Orientation orientation, QWidget *pParent = nullptr);

	TgSkinSlider(const TgSkinSlider &) = delete;
	TgSkinSlider &operator=(const TgSkinSlider &) = delete;

	QSize sizeHint() const override;

	// How many pixels on each side are treated as groove boundary.
	void setGrooveCapWidth(int px);

protected:
	void paintEvent(QPaintEvent *pEvent) override;
	void mousePressEvent(QMouseEvent *pEvent) override;
	void mouseMoveEvent(QMouseEvent *pEvent) override;
	void mouseReleaseEvent(QMouseEvent *pEvent) override;

private:
	void setValueFromMouseX(int x);
	QRect grooveRect() const;
	QRect handleSrcRect(bool pressed) const;

	QPixmap m_groove;
	QPixmap m_handle;
	int m_grooveCap = 1;
	bool m_pressed = false;
};

#endif

