/* ------------------------------------------------------------------
//  File      : tg_space_menu_widget.cpp
//  Created   : 2026-04-21
//  Purpose   : Space main menu page (sprite buttons)
// ----------------------------------------------------------------*/
#include "tg_space_menu_widget.h"

#include "common/tg_resourceprovider.h"
#include "ui/tg_imagebutton.h"

#include <QPainter>
#include <QResizeEvent>

TgSpaceMenuWidget::TgSpaceMenuWidget(TgResourceProvider *pResources, QWidget *pParent)
	: QWidget(pParent)
	, m_pResources(pResources)
{
	setFocusPolicy(Qt::NoFocus);
	setMouseTracking(true);

	if (nullptr != m_pResources)
	{
		m_bg = m_pResources->spacePixmap(QStringLiteral("SPACE_MAINMENU_BG"));
		m_startSprite = m_pResources->spacePixmap(QStringLiteral("SPACE_START"));
		m_returnSprite = m_pResources->spacePixmap(QStringLiteral("SPACE_RETURN"));
	}

	m_pStart = new TgImageButton(m_startSprite, this);
	m_pRank = new TgImageButton(m_pResources ? m_pResources->spacePixmap(QStringLiteral("SPACE_HISCORE")) : QPixmap(), this);
	m_pOption = new TgImageButton(m_pResources ? m_pResources->spacePixmap(QStringLiteral("SPACE_OPTION")) : QPixmap(), this);
	m_pExit = new TgImageButton(m_pResources ? m_pResources->spacePixmap(QStringLiteral("SPACE_EXIT")) : QPixmap(), this);

	connect(m_pStart, &TgImageButton::clicked, this, &TgSpaceMenuWidget::startClicked);
	connect(m_pRank, &TgImageButton::clicked, this, &TgSpaceMenuWidget::rankClicked);
	connect(m_pOption, &TgImageButton::clicked, this, &TgSpaceMenuWidget::optionClicked);
	connect(m_pExit, &TgImageButton::clicked, this, &TgSpaceMenuWidget::exitClicked);

	layoutButtons();
}

void TgSpaceMenuWidget::setStartAsReturn(bool isReturn)
{
	m_startIsReturn = isReturn;
	if (m_pStart)
		m_pStart->setSprite(m_startIsReturn ? m_returnSprite : m_startSprite);
	layoutButtons();
}

void TgSpaceMenuWidget::refreshLayout()
{
	layoutButtons();
	update();
}

void TgSpaceMenuWidget::resizeEvent(QResizeEvent *pEvent)
{
	QWidget::resizeEvent(pEvent);
	layoutButtons();
}

void TgSpaceMenuWidget::layoutButtons()
{
	// Reference coordinates based on 800x600:
	// Start(256,288), Rank(256,332), Option(256,376), Exit(256,420)
	const qreal baseW = 800.0;
	const qreal baseH = 600.0;
	const qreal sx = static_cast<qreal>(width()) / baseW;
	const qreal sy = static_cast<qreal>(height()) / baseH;

	auto place = [&](TgImageButton *btn, int x, int y) {
		if (!btn)
			return;
		const QSize raw = btn->sizeHint();
		// Scale X/Y independently so the button width keeps up with wide layouts,
		// avoiding exposing the background panel on the right.
		const QSize scaled(std::max(1, static_cast<int>(raw.width() * sx)),
			std::max(1, static_cast<int>(raw.height() * sy)));
		btn->setFixedSize(scaled);
		btn->move(static_cast<int>(x * sx), static_cast<int>(y * sy));
	};

	place(m_pStart, 256, 288);
	place(m_pRank, 256, 332);
	place(m_pOption, 256, 376);
	place(m_pExit, 256, 420);
}

void TgSpaceMenuWidget::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	if (!m_bg.isNull())
		painter.drawPixmap(rect(), m_bg.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

