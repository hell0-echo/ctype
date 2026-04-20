#include "tg_space_view.h"

#include "tg_space_model.h"

#include "common/tg_resourceprovider.h"

#include <algorithm>
#include <cmath>
#include <QPainter>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>

namespace
{
void drawButton(QPainter &p, const QRect &r, const QString &text)
{
	p.fillRect(r, QColor(30, 40, 80, 220));
	p.setPen(QPen(Qt::white, 2));
	p.drawRect(r);
	p.setFont(QFont("Arial", 12, QFont::Bold));
	p.drawText(r, Qt::AlignCenter, text);
}
}

TgSpaceView::TgSpaceView(TgSpaceModel *pModel, TgResourceProvider *pResources, QWidget *pParent)
	: QWidget(pParent)
	, m_pModel(pModel)
	, m_pResources(pResources)
{
	setFocusPolicy(Qt::StrongFocus);
}

void TgSpaceView::refreshLayout()
{
	layoutMenu();
}

void TgSpaceView::layoutMenu()
{
	const int w = width();
	const int h = height();
	const int bw = 200;
	const int bh = 42;
	const int cx = w / 2 - bw / 2;
	int y = h / 2 - 110;
	m_rectStart = QRect(cx, y, bw, bh);
	y += 52;
	m_rectScores = QRect(cx, y, bw, bh);
	y += 52;
	m_rectSettings = QRect(cx, y, bw, bh);
	y += 52;
	m_rectExit = QRect(cx, y, bw, bh);
}

void TgSpaceView::resizeEvent(QResizeEvent *pEvent)
{
	QWidget::resizeEvent(pEvent);
	layoutMenu();
	if (nullptr != m_pModel)
		m_pModel->setViewportSize(width(), height());
}

void TgSpaceView::paintEvent(QPaintEvent *pEvent)
{
	Q_UNUSED(pEvent);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	if (nullptr == m_pModel)
		return;

	const QPixmap bg = m_pResources->pixmap("SPACE_BACKGROUND");
	const qreal scroll = m_pModel->scrollOffset();
	const int tileH = bg.height() > 0 ? bg.height() : 600;
	const int yOff = static_cast<int>(std::fmod(scroll, static_cast<qreal>(tileH)));
	for (int y = -tileH; y < height() + tileH; y += tileH)
		painter.drawPixmap(0, y + yOff, width(), tileH, bg);

	const TgSpaceGameState st = m_pModel->state();
	if (st == TgSpaceGameState::InitialSpaceState)
	{
		painter.fillRect(rect(), QColor(0, 10, 30, 120));
		painter.setPen(Qt::white);
		painter.setFont(QFont("Arial", 22, QFont::Bold));
		painter.drawText(QRect(0, height() / 4, width(), 40), Qt::AlignCenter, QStringLiteral("Space Typing"));
		drawButton(painter, m_rectStart, QStringLiteral("Start"));
		drawButton(painter, m_rectScores, QStringLiteral("High scores"));
		drawButton(painter, m_rectSettings, QStringLiteral("Settings"));
		drawButton(painter, m_rectExit, QStringLiteral("Exit"));
		return;
	}

	const QPixmap plane = m_pResources->pixmap("SPACE_ENEMY_0");
	const QPixmap meteor = m_pResources->pixmap("SPACE_ENEMY_4");
	const QPixmap bomb = m_pResources->pixmap("SPACE_BOMB");
	const QPixmap boom = m_pResources->pixmap("SPACE_EXPLOSION_0");

	for (const TgSpaceEnemy *e : m_pModel->enemies())
	{
		const QRectF er = m_pModel->enemyRect(*e);
		if (e->m_explosionFrame >= 0)
		{
			const int f = std::min(12, std::max(0, e->m_explosionFrame));
			painter.setOpacity(0.7);
			if (boom.width() >= (f + 1) * 32)
				painter.drawPixmap(er.toAlignedRect(), boom, QRect(f * 32, 0, 32, 32));
			else
			{
				painter.setBrush(QColor(255, 140, 40, 200));
				painter.setPen(Qt::NoPen);
				painter.drawEllipse(er);
			}
			painter.setOpacity(1.0);
			continue;
		}
		const QPixmap &src = (e->m_kind == TgSpaceEnemyKind::MeteorEnemyKind) ? meteor : plane;
		painter.drawPixmap(er.toAlignedRect(), src.scaled(er.toAlignedRect().size(),
			Qt::KeepAspectRatio, Qt::SmoothTransformation));
		painter.setPen(Qt::white);
		painter.setBrush(QColor(0, 0, 0, 140));
		const QRect cap = er.toAlignedRect().adjusted(4, -22, -4, -4);
		painter.drawRoundedRect(cap, 4, 4);
		painter.drawText(cap, Qt::AlignCenter, QString(QChar::fromLatin1(e->m_letter)));
	}

	for (const TgSpaceBullet *b : m_pModel->bullets())
	{
		const QRect r(static_cast<int>(b->m_x) - 4, static_cast<int>(b->m_y) - 4, 8, 8);
		painter.drawPixmap(r, bomb.scaled(r.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	}

	const QRectF pr = m_pModel->playerRect();
	painter.setBrush(QColor(80, 200, 255));
	painter.setPen(Qt::NoPen);
	painter.drawEllipse(pr);

	painter.setPen(Qt::white);
	painter.setFont(QFont("Arial", 10));
	const QString hud = QString("Score: %1  HP: %2  Upgrade: %3s")
		.arg(m_pModel->score())
		.arg(m_pModel->hp())
		.arg(static_cast<int>(m_pModel->upgradeCountdownSec() + 0.999));
	painter.drawText(QRect(10, 8, width() - 20, 22), Qt::AlignLeft, hud);

	if (m_pModel->hasBonusWord())
	{
		const QRectF wr = m_pModel->bonusWordRect();
		painter.setBrush(QColor(255, 200, 0, 180));
		painter.setPen(Qt::black);
		painter.drawRoundedRect(wr, 6, 6);
		painter.drawText(wr, Qt::AlignCenter, m_pModel->bonusWord());
	}

	if (st == TgSpaceGameState::PausedSpaceState)
	{
		painter.fillRect(rect(), QColor(0, 0, 0, 120));
		drawButton(painter, m_rectStart, QStringLiteral("Continue"));
		drawButton(painter, m_rectScores, QStringLiteral("High scores"));
		drawButton(painter, m_rectSettings, QStringLiteral("Settings"));
		drawButton(painter, m_rectExit, QStringLiteral("Exit to launcher"));
	}
	else if (st == TgSpaceGameState::EndSpaceState)
	{
		painter.fillRect(rect(), QColor(0, 0, 0, 160));
		painter.setPen(Qt::white);
		painter.setFont(QFont("Arial", 18, QFont::Bold));
		painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("Game over"));
	}
}

void TgSpaceView::mousePressEvent(QMouseEvent *pEvent)
{
	if (nullptr == m_pModel)
	{
		QWidget::mousePressEvent(pEvent);
		return;
	}
	const QPoint pos = pEvent->pos();
	const TgSpaceGameState st = m_pModel->state();
	if (st != TgSpaceGameState::InitialSpaceState && st != TgSpaceGameState::PausedSpaceState)
	{
		QWidget::mousePressEvent(pEvent);
		return;
	}
	if (m_rectStart.contains(pos))
	{
		if (st == TgSpaceGameState::InitialSpaceState)
			emit startGameRequested();
		else
			emit continueGameRequested();
		return;
	}
	if (m_rectScores.contains(pos))
	{
		emit highScoreRequested();
		return;
	}
	if (m_rectSettings.contains(pos))
	{
		emit settingsRequested();
		return;
	}
	if (m_rectExit.contains(pos))
	{
		emit exitRequested();
		return;
	}
	QWidget::mousePressEvent(pEvent);
}

void TgSpaceView::keyPressEvent(QKeyEvent *pEvent)
{
	if (nullptr == m_pModel)
	{
		QWidget::keyPressEvent(pEvent);
		return;
	}
	if (pEvent->key() == Qt::Key_Escape)
	{
		if (m_pModel->state() == TgSpaceGameState::PlayingSpaceState)
		{
			emit pauseRequested();
			return;
		}
	}
	const QString t = pEvent->text();
	if (t.size() == 1 && t.at(0).isLetter())
	{
		const char ascii = static_cast<char>(t.at(0).toUpper().toLatin1());
		if (m_pModel->state() == TgSpaceGameState::PlayingSpaceState)
		{
			if (!m_pModel->tryFireAtLetter(ascii))
				m_pModel->applyWrongLetterPenalty();
			update();
			return;
		}
	}
	QWidget::keyPressEvent(pEvent);
}
