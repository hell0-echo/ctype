/* ------------------------------------------------------------------
//  File      : tg_imageprogressbar.cpp
//  Created   : 2026-04-27
//  Purpose   : QProgressBar drawn with background + fill pixmaps
// ----------------------------------------------------------------*/
#include "tg_imageprogressbar.h"

#include <QPainter>

TgImageProgressBar::TgImageProgressBar(const QPixmap &bg, const QPixmap &fill, QWidget *pParent)
	: QProgressBar(pParent)
	, m_bg(bg)
	, m_fill(fill)
{
	setTextVisible(false);
	setMinimum(0);
	setMaximum(100);
	setValue(0);
}

void TgImageProgressBar::setPixmaps(const QPixmap &bg, const QPixmap &fill)
{
	m_bg = bg;
	m_fill = fill;
	update();
}

void TgImageProgressBar::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

	if (!m_bg.isNull())
		painter.drawPixmap(rect(), m_bg);
	else
		painter.fillRect(rect(), QColor(240, 240, 240));

	const int minV = minimum();
	const int maxV = maximum();
	const int v = value();
	const int denom = std::max(1, (maxV - minV));
	const qreal t = std::clamp(static_cast<qreal>(v - minV) / static_cast<qreal>(denom), 0.0, 1.0);
	const int fillW = static_cast<int>(rect().width() * t);

	if (fillW <= 0 || m_fill.isNull())
		return;

	const QRect fillRect(rect().left(), rect().top(), fillW, rect().height());
	painter.save();
	painter.setClipRect(fillRect);
	painter.drawPixmap(rect(), m_fill);
	painter.restore();
}

