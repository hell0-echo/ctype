/* ------------------------------------------------------------------
//  File      : tg_imagebutton.cpp
//  Created   : 2026-04-21
//  Purpose   : Sprite-based image button (normal/hover/pressed)
// ----------------------------------------------------------------*/
#include "tg_imagebutton.h"

#include "common/tg_audiomanager.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

namespace
{
int frameCountFromSprite(const QPixmap &pm)
{
	if (pm.isNull() || pm.width() <= 0)
		return 0;
	// Resource buttons are 3 frames horizontally.
	return 3;
}
}

TgImageButton::TgImageButton(const QPixmap &sprite, QWidget *pParent)
	: QWidget(pParent)
	, m_sprite(sprite)
{
	setMouseTracking(true);
	setCursor(Qt::PointingHandCursor);
	m_disabled = makeDisabled(m_sprite);
	updateState();
}

QSize TgImageButton::sizeHint() const
{
	const int n = frameCountFromSprite(m_sprite);
	if (n <= 0)
		return QSize(80, 30);
	return QSize(m_sprite.width() / n, m_sprite.height());
}

void TgImageButton::setEnabled(bool enabled)
{
	QWidget::setEnabled(enabled);
	updateState();
	update();
}

void TgImageButton::setSprite(const QPixmap &sprite)
{
	m_sprite = sprite;
	m_disabled = makeDisabled(m_sprite);
	m_hovered = false;
	m_pressed = false;
	updateState();
	updateGeometry();
	update();
}

void TgImageButton::enterEvent(QEvent *pEvent)
{
	QWidget::enterEvent(pEvent);
	m_hovered = true;
	TgAudioManager::instance().playCommon(QStringLiteral("ANIBTN_ENTER"));
	updateState();
	update();
}

void TgImageButton::leaveEvent(QEvent *pEvent)
{
	QWidget::leaveEvent(pEvent);
	m_hovered = false;
	m_pressed = false;
	updateState();
	update();
}

void TgImageButton::mousePressEvent(QMouseEvent *pEvent)
{
	if (!isEnabled())
	{
		pEvent->ignore();
		return;
	}
	if (pEvent->button() == Qt::LeftButton)
	{
		m_pressed = true;
		TgAudioManager::instance().playCommon(QStringLiteral("ANIBTN_CLICK"));
		updateState();
		update();
	}
	QWidget::mousePressEvent(pEvent);
}

void TgImageButton::mouseReleaseEvent(QMouseEvent *pEvent)
{
	if (!isEnabled())
	{
		pEvent->ignore();
		return;
	}
	const bool wasPressed = m_pressed;
	m_pressed = false;
	updateState();
	update();
	if (wasPressed && rect().contains(pEvent->pos()) && pEvent->button() == Qt::LeftButton)
	{
		TgAudioManager::instance().playCommon(QStringLiteral("BTN_CLICK"));
		emit clicked();
	}
	QWidget::mouseReleaseEvent(pEvent);
}

void TgImageButton::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	if (m_sprite.isNull())
	{
		painter.fillRect(rect(), QColor(80, 80, 80));
		return;
	}

	if (m_state == VisualState::Disabled)
	{
		const int n = frameCountFromSprite(m_disabled);
		const QRect src(0, 0, m_disabled.width() / std::max(1, n), m_disabled.height());
		painter.drawPixmap(rect(), m_disabled, src);
		return;
	}

	const QRect src = frameRect(m_state);
	painter.drawPixmap(rect(), m_sprite, src);
}

void TgImageButton::updateState()
{
	if (!isEnabled())
	{
		m_state = VisualState::Disabled;
		return;
	}
	if (m_pressed)
		m_state = VisualState::Pressed;
	else if (m_hovered)
		m_state = VisualState::Hover;
	else
		m_state = VisualState::Normal;
}

QRect TgImageButton::frameRect(VisualState state) const
{
	const int n = frameCountFromSprite(m_sprite);
	const int frameW = (n > 0) ? (m_sprite.width() / n) : m_sprite.width();
	// Keep the same order as the reference project:
	// NORMAL=0, HOVER=1, PRESS=2
	const int idx = (state == VisualState::Pressed) ? 2 : ((state == VisualState::Hover) ? 1 : 0);
	return QRect(idx * frameW, 0, frameW, m_sprite.height());
}

QPixmap TgImageButton::makeDisabled(const QPixmap &pm) const
{
	if (pm.isNull())
		return QPixmap();
	QImage img = pm.toImage().convertToFormat(QImage::Format_ARGB32);
	for (int y = 0; y < img.height(); ++y)
	{
		QRgb *row = reinterpret_cast<QRgb *>(img.scanLine(y));
		for (int x = 0; x < img.width(); ++x)
		{
			const QColor c = QColor::fromRgba(row[x]);
			const int gray = qBound(0, static_cast<int>(0.2126 * c.red() + 0.7152 * c.green() + 0.0722 * c.blue()), 255);
			QColor g(gray, gray, gray, c.alpha());
			g.setAlpha(std::min(220, g.alpha()));
			row[x] = g.rgba();
		}
	}
	return QPixmap::fromImage(img);
}

