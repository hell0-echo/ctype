/* ------------------------------------------------------------------
//  File      : tg_space_result_dialog.cpp
//  Created   : 2026-04-27
//  Purpose   : Space game over modal dialog (frameless, custom background + sprite buttons)
// ----------------------------------------------------------------*/
#include "tg_space_result_dialog.h"

#include "common/tg_resourceprovider.h"
#include "ui/tg_imagebutton.h"

#include <QPainter>

TgSpaceResultDialog::TgSpaceResultDialog(TgResourceProvider *pResources, QWidget *pParent)
	: QDialog(pParent)
	, m_pResources(pResources)
{
	setModal(true);
	setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_DeleteOnClose, false);

	if (nullptr != m_pResources)
		m_bg = m_pResources->commonPixmap(QStringLiteral("MAIN_DLG_BG"));

	if (!m_bg.isNull())
		setFixedSize(m_bg.size());
	else
		setFixedSize(420, 260);

	if (nullptr != m_pResources)
	{
		m_pReplay = new TgImageButton(m_pResources->commonPixmap(QStringLiteral("MAIN_DLG_REPLAY")), this);
		m_pEnd = new TgImageButton(m_pResources->commonPixmap(QStringLiteral("MAIN_DLG_EXIT")), this);
	}
	else
	{
		m_pReplay = new TgImageButton(QPixmap(), this);
		m_pEnd = new TgImageButton(QPixmap(), this);
	}

	connect(m_pReplay, &TgImageButton::clicked, this, &TgSpaceResultDialog::onReplay);
	connect(m_pEnd, &TgImageButton::clicked, this, &TgSpaceResultDialog::onEnd);
	layoutControls();
}

TgSpaceResultDialog::Action TgSpaceResultDialog::action() const
{
	return m_action;
}

void TgSpaceResultDialog::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	if (!m_bg.isNull())
		painter.drawPixmap(rect(), m_bg);
	else
		painter.fillRect(rect(), QColor(240, 240, 240));

	painter.setPen(QColor(0, 0, 0));
	QFont f(QStringLiteral("SimHei"), 12, QFont::Normal);
	painter.setFont(f);
	const QString text = QString::fromUtf16(u"\u6e38\u620f\u7ed3\u675f\uff01");
	const int yTop = static_cast<int>(static_cast<qreal>(height()) * 0.2);
	const QRect textRect(18, yTop, width() - 36, height() / 2);
	painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, text);
}

void TgSpaceResultDialog::layoutControls()
{
	const QSize replaySize = m_pReplay->sizeHint();
	const QSize endSize = m_pEnd->sizeHint();
	m_pReplay->setFixedSize(replaySize);
	m_pEnd->setFixedSize(endSize);

	const int bottomMargin = 18;
	const int gap = 18;
	const int totalW = replaySize.width() + gap + endSize.width();
	const int x0 = (width() - totalW) / 2;
	const int y = height() - bottomMargin - std::max(replaySize.height(), endSize.height());
	m_pReplay->move(x0, y);
	m_pEnd->move(x0 + replaySize.width() + gap, y);
}

void TgSpaceResultDialog::onReplay()
{
	m_action = Action::Replay;
	accept();
}

void TgSpaceResultDialog::onEnd()
{
	m_action = Action::End;
	accept();
}

