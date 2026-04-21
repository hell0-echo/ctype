#include "tg_space_controller.h"

#include "ui/tg_bonuswordprovider.h"

#include <QTimer>

namespace
{
constexpr int kTimerIntervalMs = 16;
constexpr qreal kMaxStepSec = 0.05;
constexpr int kBonusIntervalMs = 10000;
}

TgSpaceController::TgSpaceController(TgSpaceModel *pModel, TgBonusWordProvider *pBonus, QObject *pParent)
	: QObject(pParent)
	, m_pModel(pModel)
	, m_pBonus(pBonus)
	, m_pTimer(new QTimer(this))
	, m_pBonusTimer(new QTimer(this))
{
	m_elapsed.start();
	m_pTimer->setInterval(kTimerIntervalMs);
	m_pBonusTimer->setInterval(kBonusIntervalMs);
	m_pBonusTimer->setSingleShot(false);
	connect(m_pTimer, &QTimer::timeout, this, &TgSpaceController::onTick);
	connect(m_pBonusTimer, &QTimer::timeout, this, &TgSpaceController::onBonusTimer);
	if (nullptr != m_pBonus)
		connect(m_pBonus, &TgBonusWordProvider::wordReady, this, &TgSpaceController::onBonusWord);
	if (nullptr != m_pModel)
		m_lastState = m_pModel->state();
}

void TgSpaceController::startLoop()
{
	m_elapsed.restart();
	m_paused = false;
	if (!m_pTimer->isActive())
		m_pTimer->start();
	if (m_bonusEnabled && nullptr != m_pModel
		&& m_pModel->state() == TgSpaceGameState::PlayingSpaceState)
	{
		if (!m_pBonusTimer->isActive())
			m_pBonusTimer->start();
	}
}

void TgSpaceController::stopLoop()
{
	m_pTimer->stop();
	m_pBonusTimer->stop();
}

void TgSpaceController::setPaused(bool paused)
{
	m_paused = paused;
}

void TgSpaceController::setBonusModeEnabled(bool on)
{
	m_bonusEnabled = on;
	if (on && m_pModel && m_pModel->state() == TgSpaceGameState::PlayingSpaceState)
	{
		if (!m_pBonusTimer->isActive())
			m_pBonusTimer->start();
	}
	else
	{
		m_pBonusTimer->stop();
	}
}

void TgSpaceController::onTick()
{
	if (nullptr == m_pModel)
		return;
	if (!m_paused && m_pModel->state() == TgSpaceGameState::PlayingSpaceState)
	{
		const qint64 ms = m_elapsed.restart();
		qreal dt = static_cast<qreal>(ms) / 1000.0;
		if (dt > kMaxStepSec)
			dt = kMaxStepSec;
		m_pModel->update(dt);
	}
	emit modelUpdated();
	const TgSpaceGameState s = m_pModel->state();
	if (s != m_lastState && s == TgSpaceGameState::EndSpaceState)
	{
		emit roundEnded();
		stopLoop();
		m_pBonusTimer->stop();
	}
	m_lastState = s;
}

void TgSpaceController::onBonusTimer()
{
	if (nullptr == m_pModel || nullptr == m_pBonus)
		return;
	if (!m_bonusEnabled)
		return;
	if (m_pModel->state() != TgSpaceGameState::PlayingSpaceState)
		return;
	if (m_pModel->hasBonusWord())
		return;
	m_pBonus->requestWordAsync();
}

void TgSpaceController::onBonusWord(const QString &word)
{
	if (nullptr == m_pModel)
		return;
	m_pModel->setBonusWord(word.trimmed().toUpper());
}
