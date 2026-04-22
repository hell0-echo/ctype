/* ------------------------------------------------------------------
//  File      : tg_imagecheckbox.cpp
//  Created   : 2026-04-21
//  Purpose   : Sprite-based checkbox button (unchecked/checked)
// ----------------------------------------------------------------*/
#include "tg_imagecheckbox.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>

TgImageCheckBox::TgImageCheckBox(const QPixmap &sprite, QWidget *pParent)
	: QWidget(pParent)
	, m_sprite(sprite)
{
	setMouseTracking(true);
	setCursor(Qt::PointingHandCursor);
}

QSize TgImageCheckBox::sizeHint() const
{
	const int n = frameCount();
	if (n <= 0)
		return QSize(22, 22);
	return QSize(m_sprite.width() / n, m_sprite.height());
}

bool TgImageCheckBox::isChecked() const
{
	return m_checked;
}

void TgImageCheckBox::setChecked(bool checked)
{
	if (m_checked == checked)
		return;
	m_checked = checked;
	update();
	emit toggled(m_checked);
}

void TgImageCheckBox::enterEvent(QEvent *pEvent)
{
	QWidget::enterEvent(pEvent);
	m_hovered = true;
	update();
}

void TgImageCheckBox::leaveEvent(QEvent *pEvent)
{
	QWidget::leaveEvent(pEvent);
	m_hovered = false;
	update();
}

void TgImageCheckBox::mouseReleaseEvent(QMouseEvent *pEvent)
{
	if (pEvent->button() == Qt::LeftButton && rect().contains(pEvent->pos()))
		setChecked(!m_checked);
	QWidget::mouseReleaseEvent(pEvent);
}

void TgImageCheckBox::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	if (m_sprite.isNull())
	{
		painter.fillRect(rect(), QColor(120, 120, 120));
		return;
	}
	painter.drawPixmap(rect(), m_sprite, frameRect(m_checked, m_hovered));
}

int TgImageCheckBox::frameCount() const
{
	if (m_sprite.isNull() || m_sprite.width() <= 0)
		return 0;
	// CHECKBOX_BUTTON has 4 frames horizontally.
	return 4;
}

QRect TgImageCheckBox::frameRect(bool checked, bool hovered) const
{
	const int n = frameCount();
	const int w = (n > 0) ? (m_sprite.width() / n) : m_sprite.width();
	const int base = checked ? 2 : 0;
	const int idx = base + (hovered ? 1 : 0);
	return QRect(idx * w, 0, w, m_sprite.height());
}

