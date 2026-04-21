#include "tg_apple_view.h"

#include "tg_apple_model.h"

#include "common/tg_resourceprovider.h"

#include <QPainter>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QResizeEvent>

TgAppleView::TgAppleView(TgAppleModel *pModel, TgResourceProvider *pResources, QWidget *pParent)
	: QWidget(pParent)
	, m_pModel(pModel)
	, m_pResources(pResources)
{
	setFocusPolicy(Qt::StrongFocus);
}

void TgAppleView::refreshMetrics()
{
	if (nullptr == m_pModel)
		return;
	const qreal ratio = devicePixelRatioF();
	Q_UNUSED(ratio);
	m_pModel->setViewportSize(width(), height());
}

void TgAppleView::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	const QPixmap bg = m_pResources->applePixmap("APPLE_BACKGROUND");
	painter.drawPixmap(rect(), bg.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

	if (nullptr == m_pModel)
		return;

	const qreal failY = m_pModel->failLineY();
	painter.setPen(QPen(QColor(255, 80, 80, 160), 2));
	painter.drawLine(0, static_cast<int>(failY), width(), static_cast<int>(failY));

	const QPixmap appleGood = m_pResources->applePixmap("APPLE_NORMAL");
	const QPixmap appleBad = m_pResources->applePixmap("APPLE_BAD");

	for (const TgAppleEntity *p : m_pModel->activeApples())
	{
		const QRectF r = m_pModel->appleRect(*p);
		const QPixmap &src = (p->m_entityState == TgAppleEntityState::NormalAppleEntity) ? appleGood : appleBad;
		painter.drawPixmap(r.toAlignedRect(), src);
		painter.setPen(Qt::black);
		painter.setFont(QFont("Arial", 14, QFont::Bold));
		painter.drawText(r, Qt::AlignCenter, QString(QChar::fromLatin1(p->m_letter)));
	}

	painter.setPen(Qt::white);
	painter.setFont(QFont("Arial", 11));
	const QString hud = QString("Score: %1  Mistakes: %2 / %3  Target: %4")
		.arg(m_pModel->score())
		.arg(m_pModel->mistakes())
		.arg(m_pModel->failLimit())
		.arg(m_pModel->winTarget());
	painter.drawText(QRect(12, 8, width() - 24, 24), Qt::AlignLeft | Qt::AlignVCenter, hud);

	const QPixmap basket = m_pResources->applePixmap("APPLE_BASKET");
	const int basketW = std::min(120, width() / 6);
	const QRect basketRect(width() - basketW - 16, height() - basketW - 16, basketW, basketW);
	painter.drawPixmap(basketRect, basket.scaled(basketRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

	const QPixmap small = m_pResources->applePixmap("APPLE_SMALL");
	const int count = m_pModel->basketAppleCount();
	for (int i = 0; i < count; ++i)
	{
		const int dx = i * 10;
		const QRect pr(basketRect.left() + 8 + dx, basketRect.top() - 18, 18, 18);
		painter.drawPixmap(pr, small.scaled(pr.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}
}

void TgAppleView::resizeEvent(QResizeEvent *pEvent)
{
	QWidget::resizeEvent(pEvent);
	refreshMetrics();
}

void TgAppleView::keyPressEvent(QKeyEvent *pEvent)
{
	if (nullptr == m_pModel)
	{
		QWidget::keyPressEvent(pEvent);
		return;
	}
	const QString t = pEvent->text();
	if (t.size() == 1)
	{
		const QChar ch = t.at(0);
		if (ch.isLetter())
		{
			if (m_pModel->state() != TgAppleGameState::PlayingAppleState)
			{
				QWidget::keyPressEvent(pEvent);
				return;
			}
			const char ascii = static_cast<char>(ch.toUpper().toLatin1());
			m_pModel->tryHitLetter(ascii);
			update();
			return;
		}
	}
	QWidget::keyPressEvent(pEvent);
}
