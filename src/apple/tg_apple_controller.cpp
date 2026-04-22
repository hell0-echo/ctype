#include "tg_apple_controller.h"

#include "tg_apple_model.h"

#include <QTimer>

namespace
{
constexpr int kTimerIntervalMs = 16;
constexpr qreal kMaxStepSec = 0.05;
}

TgAppleController::TgAppleController(TgAppleModel *pModel, QObject *pParent)
	: QObject(pParent)
	, m_pModel(pModel)
	, m_pTimer(new QTimer(this))
{
	m_elapsed.start();
	connect(m_pTimer, &QTimer::timeout, this, &TgAppleController::onTimerTick);
	m_pTimer->setInterval(kTimerIntervalMs);
	if (nullptr != m_pModel)
		m_lastState = m_pModel->state();
}

void TgAppleController::startLoop()
{
	m_elapsed.restart();
	m_paused = false;
	if (nullptr != m_pModel)
		m_lastState = m_pModel->state();
	if (!m_pTimer->isActive())
		m_pTimer->start();
}

void TgAppleController::stopLoop()
{
	m_pTimer->stop();
}

void TgAppleController::setPaused(bool paused)
{
	m_paused = paused;
}

void TgAppleController::onTimerTick()
{
	if (nullptr == m_pModel)
		return;
	const TgAppleGameState stateBefore = m_pModel->state();
	if (!m_paused && stateBefore == TgAppleGameState::PlayingAppleState)
	{
		const qint64 ms = m_elapsed.restart();
		qreal dt = static_cast<qreal>(ms) / 1000.0;
		if (dt > kMaxStepSec)
			dt = kMaxStepSec;
		m_pModel->update(dt);
	}
	emit modelUpdated();
	const TgAppleGameState stateAfter = m_pModel->state();
	if (stateAfter != m_lastState && stateAfter == TgAppleGameState::EndAppleState)
	{
		const bool win = m_pModel->score() >= m_pModel->winTarget();
		emit roundEnded(win);
		stopLoop();
	}
	m_lastState = stateAfter;
}
