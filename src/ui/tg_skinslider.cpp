/* ------------------------------------------------------------------
//  File      : tg_skinslider.cpp
//  Created   : 2026-04-21
//  Purpose   : Custom painted slider (no QSS)
// ----------------------------------------------------------------*/
#include "tg_skinslider.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

namespace
{
constexpr int kGrooveH = 4;
}

TgSkinSlider::TgSkinSlider(const QPixmap &groove, const QPixmap &handleTwoStates,
	Qt::Orientation orientation, QWidget *pParent)
	: QSlider(orientation, pParent)
	, m_groove(groove)
	, m_handle(handleTwoStates)
{
	setMouseTracking(true);
	setFocusPolicy(Qt::NoFocus);
}

QSize TgSkinSlider::sizeHint() const
{
	const int handleH = m_handle.isNull() ? 23 : m_handle.height();
	return QSize(200, std::max(32, handleH + 10));
}

void TgSkinSlider::setGrooveCapWidth(int px)
{
	m_grooveCap = std::max(0, px);
	update();
}

QRect TgSkinSlider::grooveRect() const
{
	const int y = height() / 2 - kGrooveH / 2;
	return QRect(0, y, width(), kGrooveH);
}

QRect TgSkinSlider::handleSrcRect(bool pressed) const
{
	if (m_handle.isNull())
		return QRect();
	const int halfW = m_handle.width() / 2;
	const int h = m_handle.height();
	return pressed ? QRect(halfW, 0, halfW, h) : QRect(0, 0, halfW, h);
}

void TgSkinSlider::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, false);

	const QRect g = grooveRect();
	if (!m_groove.isNull())
	{
		const int srcW = m_groove.width();
		const int srcH = m_groove.height();
		const int cap = std::min(m_grooveCap, srcW / 2);

		const QRect leftSrc(0, 0, cap, srcH);
		const QRect rightSrc(srcW - cap, 0, cap, srcH);
		const QRect centerSrc(cap, 0, std::max(1, srcW - cap * 2), srcH);

		const QRect leftDst(g.left(), g.top(), cap, g.height());
		const QRect rightDst(g.right() - cap + 1, g.top(), cap, g.height());
		const QRect centerDst(leftDst.right() + 1, g.top(),
			std::max(0, g.width() - cap * 2), g.height());

		if (cap > 0)
			painter.drawPixmap(leftDst, m_groove, leftSrc);
		if (centerDst.width() > 0)
			painter.drawTiledPixmap(centerDst, m_groove.copy(centerSrc).scaled(centerSrc.size(), Qt::IgnoreAspectRatio));
		if (cap > 0)
			painter.drawPixmap(rightDst, m_groove, rightSrc);
	}

	// Handle
	if (!m_handle.isNull())
	{
		const QRect src = handleSrcRect(m_pressed);
		const int handleW = src.width();
		const int handleH = src.height();

		const int available = std::max(0, g.width() - handleW);
		const int pos = QStyle::sliderPositionFromValue(minimum(), maximum(), value(), available, false);

		const int x = g.left() + pos;
		const int y = g.center().y() - handleH / 2;
		const QRect dst(x, y, handleW, handleH);
		painter.drawPixmap(dst, m_handle, src);
	}
}

void TgSkinSlider::setValueFromMouseX(int x)
{
	const QRect g = grooveRect();
	if (m_handle.isNull())
		return;
	const int handleW = m_handle.width() / 2;
	const int available = std::max(1, g.width() - handleW);
	const int rel = std::clamp(x - g.left(), 0, available);
	const int v = QStyle::sliderValueFromPosition(minimum(), maximum(), rel, available, false);
	setValue(v);
}

void TgSkinSlider::mousePressEvent(QMouseEvent *pEvent)
{
	if (pEvent->button() == Qt::LeftButton)
	{
		m_pressed = true;
		setValueFromMouseX(pEvent->pos().x());
		update();
	}
	QSlider::mousePressEvent(pEvent);
}

void TgSkinSlider::mouseMoveEvent(QMouseEvent *pEvent)
{
	if (m_pressed)
	{
		setValueFromMouseX(pEvent->pos().x());
		update();
	}
	QSlider::mouseMoveEvent(pEvent);
}

void TgSkinSlider::mouseReleaseEvent(QMouseEvent *pEvent)
{
	if (pEvent->button() == Qt::LeftButton)
	{
		m_pressed = false;
		update();
	}
	QSlider::mouseReleaseEvent(pEvent);
}

