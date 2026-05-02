/* ------------------------------------------------------------------
//  File      : tg_imageprogressbar.h
//  Created   : 2026-04-27
//  Purpose   : QProgressBar drawn with background + fill pixmaps
// ----------------------------------------------------------------*/
#ifndef _TYPEGAME_TG_IMAGEPROGRESSBAR_H_
#define _TYPEGAME_TG_IMAGEPROGRESSBAR_H_

#include <QPixmap>
#include <QProgressBar>

class TgImageProgressBar : public QProgressBar
{
	Q_OBJECT
public:
	explicit TgImageProgressBar(const QPixmap &bg, const QPixmap &fill, QWidget *pParent = nullptr);
	TgImageProgressBar(const TgImageProgressBar &) = delete;
	TgImageProgressBar &operator=(const TgImageProgressBar &) = delete;

	void setPixmaps(const QPixmap &bg, const QPixmap &fill);

protected:
	void paintEvent(QPaintEvent *pEvent) override;

private:
	QPixmap m_bg;
	QPixmap m_fill;
};

#endif

