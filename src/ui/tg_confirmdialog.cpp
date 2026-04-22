/* ------------------------------------------------------------------
//  File      : tg_confirmdialog.cpp
//  Created   : 2026-04-21
//  Purpose   : Modal confirm dialog with background + image buttons
// ----------------------------------------------------------------*/
#include "tg_confirmdialog.h"

#include "tg_imagebutton.h"

#include <QFont>
#include <QPainter>

TgConfirmDialog::TgConfirmDialog(const QPixmap &background, const QString &message,
	const ButtonSpec &left, const ButtonSpec &right, QWidget *pParent)
	: QDialog(pParent)
	, m_bg(background)
	, m_message(message)
	, m_left(left)
	, m_right(right)
{
	setModal(true);
	setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::Dialog);
	setAttribute(Qt::WA_DeleteOnClose, false);

	if (!m_bg.isNull())
		setFixedSize(m_bg.size());
	else
		setFixedSize(320, 200);

	m_pLeftBtn = new TgImageButton(m_left.m_sprite, this);
	m_pRightBtn = new TgImageButton(m_right.m_sprite, this);
	connect(m_pLeftBtn, &TgImageButton::clicked, this, &TgConfirmDialog::onLeftClicked);
	connect(m_pRightBtn, &TgImageButton::clicked, this, &TgConfirmDialog::onRightClicked);

	layoutControls();
}

bool TgConfirmDialog::resultAccepted() const
{
	return m_accepted;
}

void TgConfirmDialog::layoutControls()
{
	const QSize leftSize = m_pLeftBtn->sizeHint();
	const QSize rightSize = m_pRightBtn->sizeHint();
	m_pLeftBtn->setFixedSize(leftSize);
	m_pRightBtn->setFixedSize(rightSize);

	const int bottomMargin = 18;
	const int gap = 18;
	const int totalW = leftSize.width() + gap + rightSize.width();
	const int x0 = (width() - totalW) / 2;
	const int y = height() - bottomMargin - std::max(leftSize.height(), rightSize.height());
	m_pLeftBtn->move(x0, y);
	m_pRightBtn->move(x0 + leftSize.width() + gap, y);
}

void TgConfirmDialog::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
	if (!m_bg.isNull())
		painter.drawPixmap(rect(), m_bg);
	else
		painter.fillRect(rect(), QColor(210, 230, 255));

	painter.setPen(QColor(0, 0, 0));
	QFont f = painter.font();
	f.setPointSize(11);
	f.setBold(true);
	painter.setFont(f);

	const QRect textRect(18, height() / 2 - 28, width() - 36, 56);
	painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, m_message);
}

void TgConfirmDialog::onLeftClicked()
{
	m_accepted = m_left.m_accept;
	accept();
}

void TgConfirmDialog::onRightClicked()
{
	m_accepted = m_right.m_accept;
	accept();
}

