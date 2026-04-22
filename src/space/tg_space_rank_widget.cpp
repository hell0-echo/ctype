/* ------------------------------------------------------------------
//  File      : tg_space_rank_widget.cpp
//  Created   : 2026-04-21
//  Purpose   : Space rank list page (background + return button)
// ----------------------------------------------------------------*/
#include "tg_space_rank_widget.h"

#include "common/tg_resourceprovider.h"
#include "common/tg_scorestore.h"
#include "ui/tg_imagebutton.h"

#include <QPainter>
#include <QResizeEvent>

TgSpaceRankWidget::TgSpaceRankWidget(TgResourceProvider *pResources, TgScoreStore *pScores, QWidget *pParent)
	: QWidget(pParent)
	, m_pResources(pResources)
	, m_pScores(pScores)
{
	setMouseTracking(true);
	if (nullptr != m_pResources)
		m_bg = m_pResources->spacePixmap(QStringLiteral("SPACE_HISCORE_BG"));
	m_pReturn = new TgImageButton(m_pResources ? m_pResources->spacePixmap(QStringLiteral("SPACE_RETURN")) : QPixmap(), this);
	connect(m_pReturn, &TgImageButton::clicked, this, &TgSpaceRankWidget::returnClicked);
	layoutControls();
}

void TgSpaceRankWidget::refreshLayout()
{
	layoutControls();
	update();
}

void TgSpaceRankWidget::resizeEvent(QResizeEvent *pEvent)
{
	QWidget::resizeEvent(pEvent);
	layoutControls();
}

void TgSpaceRankWidget::layoutControls()
{
	// Reference: Return(500,500) on 800x600.
	const qreal baseW = 800.0;
	const qreal baseH = 600.0;
	const qreal sx = static_cast<qreal>(width()) / baseW;
	const qreal sy = static_cast<qreal>(height()) / baseH;
	const qreal s = std::min(sx, sy);

	const QSize raw = m_pReturn->sizeHint();
	m_pReturn->setFixedSize(QSize(std::max(1, static_cast<int>(raw.width() * s)),
		std::max(1, static_cast<int>(raw.height() * s))));
	m_pReturn->move(static_cast<int>(500 * sx), static_cast<int>(500 * sy));
}

void TgSpaceRankWidget::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	if (!m_bg.isNull())
		painter.drawPixmap(rect(), m_bg.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

	if (nullptr == m_pScores)
		return;
	const auto entries = m_pScores->topTen();

	painter.setPen(Qt::white);
	QFont f(QStringLiteral("Arial"), 12, QFont::Bold);
	painter.setFont(f);

	// Simple text list roughly centered; the reference uses a child widget, but visually equivalent.
	const int x = static_cast<int>(width() * 0.34);
	int y = static_cast<int>(height() * 0.28);
	const int lineH = 26;
	int rank = 1;
	for (const auto &e : entries)
	{
		const QString line = QStringLiteral("%1. %2  %3").arg(rank).arg(e.m_name).arg(e.m_score);
		painter.drawText(QPoint(x, y), line);
		y += lineH;
		rank++;
		if (rank > 10)
			break;
	}
}

