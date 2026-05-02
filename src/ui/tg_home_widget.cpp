/* ------------------------------------------------------------------
//  File      : tg_home_widget.cpp
//  Created   : 2026-04-22
//  Purpose   : Custom painted launcher (Apple/Space)
// ----------------------------------------------------------------*/
#include "tg_home_widget.h"

#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QFontDatabase>
#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

namespace
{
QString resPath(const QString &name)
{
	return QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("res") + QDir::separator() + name;
}

QRect centeredRectKeepAspect(const QSize &src, const QRect &bounds)
{
	if (src.width() <= 0 || src.height() <= 0 || bounds.width() <= 0 || bounds.height() <= 0)
		return QRect();
	const QSize fit = src.scaled(bounds.size(), Qt::KeepAspectRatio);
	const int x = bounds.x() + (bounds.width() - fit.width()) / 2;
	const int y = bounds.y() + (bounds.height() - fit.height()) / 2;
	return QRect(QPoint(x, y), fit);
}

void drawSoftShadow(QPainter &p, const QRect &r, int radius, int blur)
{
	// Cheap soft shadow via multiple strokes.
	for (int i = 0; i < blur; ++i)
	{
		const int a = qMax(0, 70 - i * (70 / qMax(1, blur)));
		p.setPen(QPen(QColor(0, 0, 0, a), 1.0));
		p.setBrush(Qt::NoBrush);
		p.drawRoundedRect(r.adjusted(-i, -i, i, i), radius + i, radius + i);
	}
}

QFont uiFont(int pointSize, bool bold)
{
	// Prefer common CJK fonts on Windows; fall back gracefully.
	QStringList families;
	families << QStringLiteral("Microsoft YaHei UI")
		<< QStringLiteral("Microsoft YaHei")
		<< QStringLiteral("SimHei")
		<< QStringLiteral("Arial");
	for (const QString &fam : families)
	{
		if (QFontDatabase().families().contains(fam))
		{
			QFont f(fam, pointSize);
			f.setBold(bold);
			return f;
		}
	}
	QFont f;
	f.setPointSize(pointSize);
	f.setBold(bold);
	return f;
}
}

TgHomeWidget::TgHomeWidget(QWidget *pParent)
	: QWidget(pParent)
{
	setMouseTracking(true);
	loadPixmaps();
	updateLayout();
}

void TgHomeWidget::loadPixmaps()
{
	m_applePm = QPixmap(resPath(QStringLiteral("apple.png")));
	m_spacePm = QPixmap(resPath(QStringLiteral("space.png")));
}

void TgHomeWidget::resizeEvent(QResizeEvent *pEvent)
{
	QWidget::resizeEvent(pEvent);
	updateLayout();
}

void TgHomeWidget::updateLayout()
{
	const QRect rc = rect();
	if (rc.isEmpty())
		return;

	// Layout is based on the reference launcher screenshot (top bar + content area).
	const int w = rc.width();
	const int h = rc.height();

	// Top header (blue).
	const int headerH = qMax(92, h / 8);
	// Leave room for the "经典游戏" section title + divider.
	const int sectionTitleH = 24;
	const int sectionGap = 12;
	const int contentTop = headerH + sectionGap + sectionTitleH + 18;

	// Card grid inside content area.
	const int sidePad = qMax(36, w / 14);
	const int gap = qMax(36, w / 20);
	const int cardW = qMax(220, qMin(320, (w - sidePad * 2 - gap) / 2));
	const int cardH = qMax(176, qMin(210, h / 3));

	const int y = contentTop + qMax(0, (h - contentTop - cardH) / 3);
	const int x0 = sidePad;

	m_appleTile = QRect(x0, y, cardW, cardH);
	m_spaceTile = QRect(x0 + cardW + gap, y, cardW, cardH);

	// Image framed area (like the reference: white card with inner image + title below).
	const int framePad = qMax(10, cardW / 20);
	const int titleH = 28;

	const QRect appleFrame = m_appleTile.adjusted(framePad, framePad, -framePad, -(framePad + titleH));
	const QRect spaceFrame = m_spaceTile.adjusted(framePad, framePad, -framePad, -(framePad + titleH));

	// Normalize perceived size: fit both images into same inner bounds and keep aspect.
	// Add a slight top bias to mimic the screenshot spacing.
	const QRect appleInner = appleFrame.adjusted(10, 10, -10, -10);
	const QRect spaceInner = spaceFrame.adjusted(10, 10, -10, -10);
	m_appleImageRect = centeredRectKeepAspect(m_applePm.size(), appleInner);
	m_spaceImageRect = centeredRectKeepAspect(m_spacePm.size(), spaceInner);
	if (!m_appleImageRect.isEmpty())
		m_appleImageRect.translate(0, -2);
	if (!m_spaceImageRect.isEmpty())
		m_spaceImageRect.translate(0, -2);

	m_appleTitleRect = QRect(m_appleTile.left(), m_appleTile.bottom() - titleH, m_appleTile.width(), titleH);
	m_spaceTitleRect = QRect(m_spaceTile.left(), m_spaceTile.bottom() - titleH, m_spaceTile.width(), titleH);

	update();
}

TgHomeWidget::HoverTarget TgHomeWidget::hitTest(const QPoint &pt) const
{
	if (m_appleTile.contains(pt))
		return HoverTarget::Apple;
	if (m_spaceTile.contains(pt))
		return HoverTarget::Space;
	return HoverTarget::None;
}

void TgHomeWidget::mouseMoveEvent(QMouseEvent *pEvent)
{
	const HoverTarget h = hitTest(pEvent->pos());
	if (h != m_hover)
	{
		m_hover = h;
		setCursor(m_hover == HoverTarget::None ? Qt::ArrowCursor : Qt::PointingHandCursor);
		update();
	}
	QWidget::mouseMoveEvent(pEvent);
}

void TgHomeWidget::leaveEvent(QEvent *pEvent)
{
	m_hover = HoverTarget::None;
	setCursor(Qt::ArrowCursor);
	update();
	QWidget::leaveEvent(pEvent);
}

void TgHomeWidget::mouseReleaseEvent(QMouseEvent *pEvent)
{
	if (pEvent->button() == Qt::LeftButton)
	{
		const HoverTarget h = hitTest(pEvent->pos());
		if (h == HoverTarget::Apple)
			emit appleRequested();
		else if (h == HoverTarget::Space)
			emit spaceRequested();
	}
	QWidget::mouseReleaseEvent(pEvent);
}

void TgHomeWidget::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);

	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setRenderHint(QPainter::TextAntialiasing, true);
	p.setRenderHint(QPainter::SmoothPixmapTransform, true);

	// --- Background: mimic the reference launcher (blue header + light content area) ---
	const int headerH = qMax(92, height() / 8);

	// Main content background.
	p.fillRect(rect(), QColor(245, 248, 252));

	// Top blue header.
	{
		QRect header(0, 0, width(), headerH);
		QLinearGradient g(header.topLeft(), header.bottomLeft());
		g.setColorAt(0.0, QColor(84, 167, 238));
		g.setColorAt(0.55, QColor(70, 152, 230));
		g.setColorAt(1.0, QColor(60, 140, 220));
		p.fillRect(header, g);

		// Subtle pattern: translucent bubbles + ABC outlines.
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(255, 255, 255, 22));
		for (int i = 0; i < 18; ++i)
		{
			const int r = 10 + (i * 7) % 42;
			const int x = (i * 83) % qMax(1, width());
			const int y = (i * 29) % qMax(1, headerH);
			p.drawEllipse(QPoint(x, y), r, r);
		}

		p.setPen(QPen(QColor(255, 255, 255, 30), 2.0));
		QFont abcf = uiFont(qMax(18, headerH / 4), true);
		p.setFont(abcf);
		p.drawText(QRect(width() - 240, 6, 220, headerH - 12), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("ABC"));

		// App title.
		p.setPen(QColor(255, 255, 255, 235));
		QFont titleF = uiFont(qMax(18, headerH / 4), true);
		p.setFont(titleF);
		p.drawText(QRect(26, 0, width() - 52, headerH), Qt::AlignLeft | Qt::AlignVCenter,
			QString::fromUtf16(u"\u91d1\u5c71\u6253\u5b57\u901a 2016"));
	}

	// Section title: “经典游戏”
	{
		p.setPen(QColor(30, 40, 50));
		p.setFont(uiFont(10, true));
		const int y = headerH + 12;
		p.drawText(QRect(24, y, width() - 48, 20), Qt::AlignLeft | Qt::AlignVCenter,
			QString::fromUtf16(u"\u7ecf\u5178\u6e38\u620f"));
		p.setPen(QPen(QColor(210, 220, 230), 1.0));
		p.drawLine(24, y + 24, width() - 24, y + 24);
	}

	// --- Tiles: two cards with inner image + title under ---
	auto drawTile = [&](const QRect &tile, const QRect &imgRect, const QRect &titleRect,
		const QPixmap &pm, const QString &title, bool hovered) {
		const QRect card = tile;
		const QRect frame = card.adjusted(10, 10, -10, -32);
		const int radius = 6;

		// Outer card shadow + border.
		drawSoftShadow(p, card.adjusted(0, 0, 0, 0), radius, hovered ? 7 : 5);

		p.setPen(QPen(QColor(210, 220, 230), 1.0));
		p.setBrush(QColor(255, 255, 255, 255));
		p.drawRoundedRect(card, radius, radius);

		// Inner frame (slightly inset) like the screenshot's tile.
		p.setPen(QPen(hovered ? QColor(160, 200, 235) : QColor(225, 232, 240), hovered ? 2.0 : 1.0));
		p.setBrush(QColor(255, 255, 255));
		p.drawRect(frame);

		// Image.
		if (!pm.isNull() && !imgRect.isEmpty())
		{
			// Keep aspect and normalize across different source sizes (already computed in updateLayout).
			p.drawPixmap(imgRect, pm);
		}
		else
		{
			p.setPen(QColor(140, 140, 140));
			p.setFont(uiFont(10, false));
			p.drawText(frame.adjusted(8, 8, -8, -8), Qt::AlignCenter, QStringLiteral("Missing image"));
		}

		// Title under the card (center aligned).
		p.setPen(QColor(40, 50, 60));
		p.setFont(uiFont(10, false));
		p.drawText(titleRect, Qt::AlignHCenter | Qt::AlignVCenter, title);
	};

	drawTile(m_appleTile, m_appleImageRect, m_appleTitleRect, m_applePm,
		QString::fromUtf16(u"\u62ef\u6551\u82f9\u679c"), m_hover == HoverTarget::Apple);
	drawTile(m_spaceTile, m_spaceImageRect, m_spaceTitleRect, m_spacePm,
		QString::fromUtf16(u"\u592a\u7a7a\u5927\u6218"), m_hover == HoverTarget::Space);
}

