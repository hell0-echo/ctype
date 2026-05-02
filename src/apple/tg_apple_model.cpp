#include "tg_apple_model.h"

#include "common/tg_math.h"
#include "common/tg_audiomanager.h"

#include <algorithm>
#include <cstdlib>
#include <QtGlobal>

namespace
{
constexpr qreal kAppleSize = 48.0;
constexpr qreal kBrokenFallSpeed = 120.0;

qreal randomRange(qreal lo, qreal hi)
{
	const qreal t = static_cast<qreal>(qrand()) / static_cast<qreal>(RAND_MAX);
	return lo + (hi - lo) * t;
}

int randomInt(int loInclusive, int hiInclusive)
{
	return loInclusive + (qrand() % (hiInclusive - loInclusive + 1));
}
}

TgAppleModel::TgAppleModel() = default;

void TgAppleModel::setViewportSize(int width, int height)
{
	m_viewW = std::max(100, width);
	m_viewH = std::max(100, height);
}

void TgAppleModel::setLevel(int level)
{
	m_level = static_cast<int>(tg::clamp(static_cast<qreal>(level), 0.0, 9.0));
}

void TgAppleModel::setFailLimit(int value)
{
	m_failLimit = std::clamp(value, 5, 50);
}

void TgAppleModel::setWinTarget(int value)
{
	m_winTarget = std::clamp(value, 5, 900);
}

void TgAppleModel::setMaxApplesOnScreen(int value)
{
	m_maxOnScreen = std::clamp(value, 1, 5);
}

int TgAppleModel::level() const
{
	return m_level;
}

int TgAppleModel::score() const
{
	return m_score;
}

int TgAppleModel::mistakes() const
{
	return m_mistakes;
}

int TgAppleModel::winTarget() const
{
	return m_winTarget;
}

int TgAppleModel::failLimit() const
{
	return m_failLimit;
}

int TgAppleModel::maxApplesOnScreen() const
{
	return m_maxOnScreen;
}

TgAppleGameState TgAppleModel::state() const
{
	return m_state;
}

void TgAppleModel::setState(TgAppleGameState state)
{
	m_state = state;
}

void TgAppleModel::resetIdle()
{
	for (TgAppleEntity *p : m_active)
		m_pool.release(p);
	m_active.clear();
	m_score = 0;
	m_mistakes = 0;
	m_spawnCooldown = 0.0;
	m_basketApples = 0;
	m_state = TgAppleGameState::IdleAppleState;
}

void TgAppleModel::startNewRound()
{
	for (TgAppleEntity *p : m_active)
		m_pool.release(p);
	m_active.clear();
	m_score = 0;
	m_mistakes = 0;
	m_basketApples = 0;
	m_state = TgAppleGameState::PlayingAppleState;
	m_spawnCooldown = 0.15;
}

void TgAppleModel::update(qreal deltaSec)
{
	if (m_state != TgAppleGameState::PlayingAppleState)
		return;

	const qreal failY = failLineY();

	for (auto it = m_active.begin(); it != m_active.end();)
	{
		TgAppleEntity *p = *it;
		if (p->m_entityState == TgAppleEntityState::NormalAppleEntity)
		{
			p->m_y += p->m_speedY * deltaSec;
			const QRectF r = appleRect(*p);
			if (r.bottom() >= failY)
			{
				p->m_entityState = TgAppleEntityState::BrokenAppleEntity;
				p->m_brokenAge = 0.0;
				m_mistakes++;
				if (m_mistakes >= m_failLimit)
					m_state = TgAppleGameState::EndAppleState;
			}
		}
		else
		{
			p->m_brokenAge += deltaSec;
			p->m_y += kBrokenFallSpeed * deltaSec;
		}

		const bool offscreen = p->m_y > m_viewH + kAppleSize * 2.0;
		const bool brokenExpired = p->m_entityState == TgAppleEntityState::BrokenAppleEntity
			&& p->m_brokenAge > 0.45;
		if (offscreen || brokenExpired)
		{
			releaseEntity(p);
			it = m_active.erase(it);
			continue;
		}
		++it;
	}

	if (m_state == TgAppleGameState::PlayingAppleState && m_score >= m_winTarget)
		m_state = TgAppleGameState::EndAppleState;

	if (m_state == TgAppleGameState::PlayingAppleState)
	{
		m_spawnCooldown -= deltaSec;
		spawnAppleIfNeeded();
	}
}


bool TgAppleModel::tryHitLetter(char letter)
{
	if (m_state != TgAppleGameState::PlayingAppleState)
		return false;
	const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(letter)));
	if (upper < 'A' || upper > 'Z')
		return false;
	for (auto it = m_active.begin(); it != m_active.end(); ++it)
	{
		TgAppleEntity *p = *it;
		if (p->m_entityState != TgAppleEntityState::NormalAppleEntity)
			continue;
		if (p->m_letter != upper)
			continue;
		releaseEntity(p);
		m_active.erase(it);
		m_score++;
		TgAudioManager::instance().playApple(QStringLiteral("APPLE_IN"));
		updateBasketVisual();
		if (m_score >= m_winTarget)
			m_state = TgAppleGameState::EndAppleState;
		return true;
	}
	return false;
}

qreal TgAppleModel::failLineY() const
{
	return m_viewH * 0.7;
}

QRectF TgAppleModel::appleRect(const TgAppleEntity &entity) const
{
	const qreal cx = entity.m_x;
	const qreal cy = entity.m_y;
	return QRectF(cx - kAppleSize * 0.5, cy - kAppleSize * 0.5, kAppleSize, kAppleSize);
}

int TgAppleModel::basketAppleCount() const
{
	return m_basketApples;
}

const std::vector<TgAppleEntity *> &TgAppleModel::activeApples() const
{
	return m_active;
}

void TgAppleModel::spawnAppleIfNeeded()
{
	if (m_spawnCooldown > 0.0)
		return;
	if (static_cast<int>(m_active.size()) >= m_maxOnScreen)
	{
		m_spawnCooldown = randomRange(0.15, 0.4);
		return;
	}
	m_spawnCooldown = randomRange(0.35, 1.1);
	TgAppleEntity *p = m_pool.acquire();
	if (nullptr == p)
		return;
	// Apple falling speed has exactly 10 discrete levels (0..9), determined solely by game level.
	// This keeps the speed set consistent and avoids per-apple random speed at the same level.
	const qreal speedBase = 60.0;
	const qreal speedStep = 28.0;
	p->m_speedY = speedBase + static_cast<qreal>(m_level) * speedStep;
	p->m_letter = pickUnusedLetter();
	p->m_entityState = TgAppleEntityState::NormalAppleEntity;
	p->m_brokenAge = 0.0;
	const qreal margin = kAppleSize;
	p->m_x = randomRange(margin, static_cast<qreal>(m_viewW) - margin);
	p->m_y = -kAppleSize;
	m_active.push_back(p);
}

bool TgAppleModel::isLetterUsed(char letter) const
{
	for (const TgAppleEntity *p : m_active)
	{
		if (p->m_entityState != TgAppleEntityState::NormalAppleEntity)
			continue;
		if (p->m_letter == letter)
			return true;
	}
	return false;
}

char TgAppleModel::pickUnusedLetter() const
{
	for (int attempt = 0; attempt < 64; ++attempt)
	{
		const char c = static_cast<char>('A' + (qrand() % 26));
		if (!isLetterUsed(c))
			return c;
	}
	return 'A';
}

void TgAppleModel::releaseEntity(TgAppleEntity *pEntity)
{
	m_pool.release(pEntity);
}

void TgAppleModel::updateBasketVisual()
{
	const int step = std::max(1, m_winTarget / 8);
	const int next = m_score / step;
	m_basketApples = std::min(8, next);
}
