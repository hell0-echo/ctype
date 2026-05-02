/* ------------------------------------------------------------------
//  File      : tg_apple_result_dialog.cpp
//  Created   : 2026-04-23
//  Purpose   : Apple win/lose modal dialog (custom background + sprite buttons)
// ----------------------------------------------------------------*/
#include "tg_apple_result_dialog.h"

#include "common/tg_resourceprovider.h"
#include "ui/tg_imagebutton.h"

#include <QPainter>

TgAppleResultDialog::TgAppleResultDialog(TgResourceProvider *pResources, bool isWin, QWidget *pParent)
	: QDialog(pParent)
	, m_pResources(pResources)
	, m_isWin(isWin)
{
	setModal(true);
	// Frameless: match reference dialogs (no title bar / close button).
	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_DeleteOnClose, false);

	if (nullptr != m_pResources)
		m_bg = m_pResources->applePixmap(QStringLiteral("APPLE_DLG_BG"));

	if (!m_bg.isNull())
		setFixedSize(m_bg.size());
	else
		setFixedSize(420, 260);

	// Buttons (sprites contain normal/hover/pressed frames; disabled not used here)
	if (nullptr != m_pResources)
	{
		m_pReplay = new TgImageButton(m_pResources->applePixmap(QStringLiteral("APPLE_DLG_REPLAY")), this);
		m_pEnd = new TgImageButton(m_pResources->applePixmap(QStringLiteral("APPLE_DLG_END")), this);
		if (m_isWin)
			m_pNext = new TgImageButton(m_pResources->applePixmap(QStringLiteral("APPLE_DLG_NEXT")), this);
	}
	else
	{
		m_pReplay = new TgImageButton(QPixmap(), this);
		m_pEnd = new TgImageButton(QPixmap(), this);
		if (m_isWin)
			m_pNext = new TgImageButton(QPixmap(), this);
	}

	connect(m_pReplay, &TgImageButton::clicked, this, &TgAppleResultDialog::onReplay);
	connect(m_pEnd, &TgImageButton::clicked, this, &TgAppleResultDialog::onEnd);
	if (m_pNext)
		connect(m_pNext, &TgImageButton::clicked, this, &TgAppleResultDialog::onNext);

	layoutControls();
}

TgAppleResultDialog::Action TgAppleResultDialog::action() const
{
	return m_action;
}

void TgAppleResultDialog::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	if (!m_bg.isNull())
		painter.drawPixmap(rect(), m_bg);
	else
		painter.fillRect(rect(), QColor(240, 240, 240));

	// Draw missing reference text (not embedded in the background assets).
	painter.setPen(QColor(0, 0, 0));
	QFont f(QStringLiteral("SimHei"), 12, QFont::Normal);
	painter.setFont(f);
	const QString text = m_isWin
		? QString::fromUtf16(u"\u606d\u559c\uff0c\u60a8\u901a\u8fc7\u4e86\uff01")
		: QString::fromUtf16(u"\u60a8\u8ba4\u8f93\u5427\uff01");
	const int yTop = static_cast<int>(static_cast<qreal>(height()) * 0.2);
	const QRect textRect(18, yTop, width() - 36, height() / 2);
	painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
}

void TgAppleResultDialog::layoutControls()
{
	const QSize replaySize = m_pReplay->sizeHint();
	const QSize endSize = m_pEnd->sizeHint();
	const QSize nextSize = m_pNext ? m_pNext->sizeHint() : QSize();

	m_pReplay->setFixedSize(replaySize);
	m_pEnd->setFixedSize(endSize);
	if (m_pNext)
		m_pNext->setFixedSize(nextSize);

	const int bottomMargin = 18;
	const int gap = 18;
	const int y = height() - bottomMargin - std::max({ replaySize.height(), endSize.height(), nextSize.height() });

	if (m_isWin && m_pNext)
	{
		const int totalW = replaySize.width() + gap + nextSize.width() + gap + endSize.width();
		const int x0 = (width() - totalW) / 2;
		m_pReplay->move(x0, y);
		m_pNext->move(x0 + replaySize.width() + gap, y);
		m_pEnd->move(x0 + replaySize.width() + gap + nextSize.width() + gap, y);
	}
	else
	{
		const int totalW = replaySize.width() + gap + endSize.width();
		const int x0 = (width() - totalW) / 2;
		m_pReplay->move(x0, y);
		m_pEnd->move(x0 + replaySize.width() + gap, y);
	}
}

void TgAppleResultDialog::onReplay()
{
	m_action = Action::Replay;
	accept();
}

void TgAppleResultDialog::onNext()
{
	m_action = Action::Next;
	accept();
}

void TgAppleResultDialog::onEnd()
{
	m_action = Action::End;
	accept();
}

