#include "tg_apple_view.h"

#include "tg_apple_model.h"

#include "common/tg_resourceprovider.h"
#include "common/tg_audiomanager.h"

#include <algorithm>
#include <vector>
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

	const QPixmap appleGood = m_pResources->applePixmap("APPLE_NORMAL");
	const QPixmap appleBad = m_pResources->applePixmap("APPLE_BAD");

	for (const TgAppleEntity *p : m_pModel->activeApples())
	{
		const QRectF r = m_pModel->appleRect(*p);
		const QPixmap &src = (p->m_entityState == TgAppleEntityState::NormalAppleEntity) ? appleGood : appleBad;
		painter.drawPixmap(r.toAlignedRect(), src);
		// Only show the letter for normal apples; broken apples should not display letters.
		if (p->m_entityState == TgAppleEntityState::NormalAppleEntity)
		{
			painter.setPen(Qt::black);
			painter.setFont(QFont("Arial", 14, QFont::Bold));
			painter.drawText(r, Qt::AlignCenter, QString(QChar::fromLatin1(p->m_letter)));
		}
	}

	const QPixmap basket = m_pResources->applePixmap("APPLE_BASKET");
	const int basketW = std::min(120, width() / 6);
	const QRect basketRect(width() - basketW - 16, height() - basketW - 16, basketW, basketW);

	// Draw apples first, then basket on top (basket layer must be above apples).
	const QPixmap small = m_pResources->applePixmap("APPLE_SMALL");
	const int count = std::clamp(m_pModel->basketAppleCount(), 0, 8);
	if (count > 0 && !small.isNull())
	{
		// 3 layers: bottom=3, middle=2, top=3.
		// Apple size is set to 60% of the previous 400% size (64px * 0.6 ~= 38px),
		// so it stays visible inside the basket.
		const int appleSize = 38;
		const int spacingX = static_cast<int>(appleSize * 0.72);
		// Vertical gap with a slight expansion (+10%).
		const int spacingY = static_cast<int>(appleSize * 0.26 * 1.10);
		const int cx = basketRect.center().x();
		// Shift the whole stack slightly to the left (~5% of basket width).
		const int rightShift = -static_cast<int>(static_cast<qreal>(basketRect.width()) * 0.05);
		// Move the whole stack slightly upward inside the basket.
		const int bottomPad = static_cast<int>(basketRect.height() * 0.30);
		const int y0 = basketRect.bottom() - bottomPad - appleSize;

		auto rowX0 = [&](int n) -> int {
			const int rowW = appleSize + (n - 1) * spacingX;
			return (cx + rightShift) - rowW / 2;
		};

		struct Slot { int m_x; int m_y; };

		std::vector<Slot> rowBottom;
		std::vector<Slot> rowMid;
		std::vector<Slot> rowTop;
		rowBottom.reserve(3);
		rowMid.reserve(2);
		rowTop.reserve(3);

		// Bottom row (3) - layer 2
		{
			const int x0 = rowX0(3);
			for (int i = 0; i < 3; ++i)
				rowBottom.push_back(Slot{ x0 + i * spacingX, y0 });
		}
		// Middle row (2) - layer 3
		{
			const int x0 = rowX0(2);
			for (int i = 0; i < 2; ++i)
				rowMid.push_back(Slot{ x0 + i * spacingX, y0 - spacingY });
		}
		// Top row (3) - layer 4
		{
			const int x0 = rowX0(3);
			for (int i = 0; i < 3; ++i)
				rowTop.push_back(Slot{ x0 + i * spacingX, y0 - 2 * spacingY });
		}

		auto drawSlot = [&](const Slot &s) {
			const QRect pr(s.m_x, s.m_y, appleSize, appleSize);
			painter.drawPixmap(pr, small.scaled(pr.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		};

		// Draw order controls z-layer among apples.
		// To match the reference perspective, bottom row should appear above middle/top.
		// Basket is drawn last and remains the top-most layer.
		for (int i = 0; i < std::min(std::max(0, count - 5), 3); ++i)
			drawSlot(rowTop[i]);
		for (int i = 0; i < std::min(std::max(0, count - 3), 2); ++i)
			drawSlot(rowMid[i]);
		for (int i = 0; i < std::min(count, 3); ++i)
			drawSlot(rowBottom[i]);
	}

	painter.drawPixmap(basketRect, basket.scaled(basketRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
			TgAudioManager::instance().playCommon(QStringLiteral("TYPE"));
			m_pModel->tryHitLetter(ascii);
			update();
			return;
		}
	}
	QWidget::keyPressEvent(pEvent);
}
