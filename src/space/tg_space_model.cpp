#include "tg_space_model.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <QtGlobal>
#include <QString>

#include "common/tg_audiomanager.h"

namespace
{
constexpr qreal kPlayerHalfW = 40.0;
constexpr qreal kPlayerHalfH = 24.0;
constexpr qreal kEnemyHalf = 28.0;
constexpr qreal kBulletSpeed = 520.0;
constexpr qreal kSpawnIntervalMin = 0.35;
constexpr qreal kSpawnIntervalMax = 1.2;

qreal frand(qreal lo, qreal hi)
{
	const qreal t = static_cast<qreal>(qrand()) / static_cast<qreal>(RAND_MAX);
	return lo + (hi - lo) * t;
}

bool rectFullyInsideView(const QRectF &r, int w, int h)
{
	return r.left() >= 0.0 && r.top() >= 0.0 && r.right() <= w && r.bottom() <= h;
}
}

TgSpaceModel::TgSpaceModel() = default;

void TgSpaceModel::setViewportSize(int width, int height)
{
	m_viewW = std::max(200, width);
	m_viewH = std::max(200, height);
	m_playerX = m_viewW * 0.5;
}

void TgSpaceModel::setMaxEnemies(int value)
{
	m_maxEnemies = std::clamp(value, 1, 10);
}

void TgSpaceModel::setSpeedLevel(int value)
{
	m_speedLevel = std::clamp(value, 1, 10);
}

void TgSpaceModel::setUpgradeIntervalSec(int value)
{
	m_upgradeIntervalSec = std::max(10, value);
}

void TgSpaceModel::setBonusMode(bool on)
{
	m_bonusMode = on;
}

int TgSpaceModel::score() const
{
	return m_score;
}

int TgSpaceModel::hp() const
{
	return m_hp;
}

int TgSpaceModel::maxEnemies() const
{
	return m_maxEnemies;
}

int TgSpaceModel::speedLevel() const
{
	return m_speedLevel;
}

int TgSpaceModel::upgradeIntervalSec() const
{
	return m_upgradeIntervalSec;
}

bool TgSpaceModel::bonusMode() const
{
	return m_bonusMode;
}

qreal TgSpaceModel::upgradeCountdownSec() const
{
	return m_upgradeCountdown;
}

TgSpaceGameState TgSpaceModel::state() const
{
	return m_state;
}

void TgSpaceModel::setState(TgSpaceGameState state)
{
	m_state = state;
}

void TgSpaceModel::startFromMenu()
{
	m_state = TgSpaceGameState::PlayingSpaceState;
	m_score = 0;
	m_hp = 18;
	m_playerX = m_viewW * 0.5;
	m_playerDir = 1.0;
	m_scroll = 0.0;
	m_upgradeCountdown = static_cast<qreal>(m_upgradeIntervalSec);
	m_bonusActive = false;
	m_bonusWord.clear();
	m_bonusProgress = 0;
	m_spawnCooldown = 0.6;

	for (TgSpaceEnemy *p : m_enemies)
		m_enemyPool.release(p);
	m_enemies.clear();
	for (TgSpaceBullet *b : m_bullets)
		m_bulletPool.release(b);
	m_bullets.clear();
	for (TgSpaceExplosion *e : m_explosions)
		m_explosionPool.release(e);
	m_explosions.clear();
}

void TgSpaceModel::resumeFromPause()
{
	if (m_state == TgSpaceGameState::PausedSpaceState)
		m_state = TgSpaceGameState::PlayingSpaceState;
}

void TgSpaceModel::pauseToMenu()
{
	if (m_state == TgSpaceGameState::PlayingSpaceState)
		m_state = TgSpaceGameState::PausedSpaceState;
}

void TgSpaceModel::resetToInitial()
{
	m_state = TgSpaceGameState::InitialSpaceState;
	for (TgSpaceEnemy *p : m_enemies)
		m_enemyPool.release(p);
	m_enemies.clear();
	for (TgSpaceBullet *b : m_bullets)
		m_bulletPool.release(b);
	m_bullets.clear();
	for (TgSpaceExplosion *e : m_explosions)
		m_explosionPool.release(e);
	m_explosions.clear();
	m_bonusActive = false;
}

void TgSpaceModel::update(qreal deltaSec)
{
	if (m_state != TgSpaceGameState::PlayingSpaceState)
		return;

	m_scroll += 40.0 * deltaSec;
	if (m_scroll > 100000.0)
		m_scroll = 0.0;

	movePlayer(deltaSec);
	moveEnemies(deltaSec);
	moveBullets(deltaSec);
	pruneExplosions(deltaSec);
	tickDifficulty(deltaSec);
	tickBonusWord(deltaSec);
	spawnEnemyIfNeeded(deltaSec);

	if (m_hp <= 0)
		m_state = TgSpaceGameState::EndSpaceState;
}

bool TgSpaceModel::tryFireAtLetter(char letter)
{
	if (m_state != TgSpaceGameState::PlayingSpaceState)
		return false;
	const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(letter)));
	if (upper < 'A' || upper > 'Z')
		return false;

	bool handled = false;
	// Bonus word and enemy matching should run concurrently: one keypress can progress the bonus word
	// and also fire at a matching enemy.
	if (m_bonusActive && !m_bonusWord.isEmpty())
		handled = tryProgressBonusWord(upper) || handled;

	for (TgSpaceEnemy *p : m_enemies)
	{
		if (p->m_explosionFrame >= 0)
			continue;
		if (p->m_letter != upper)
			continue;
		const QRectF er = enemyRect(*p);
		if (!rectFullyInsideView(er, m_viewW, m_viewH))
			continue;
		if (!p->m_hittable)
			continue;
		// Only one tracking bullet per enemy at a time.
		for (const TgSpaceBullet *existing : m_bullets)
		{
			if (nullptr != existing && existing->m_pTarget == p)
				return true;
		}
		TgSpaceBullet *b = m_bulletPool.acquire();
		if (nullptr == b)
			return handled;
		b->m_x = m_playerX;
		b->m_y = playerRect().top();
		b->m_pTarget = p;
		m_bullets.push_back(b);
		TgAudioManager::instance().playSpace(QStringLiteral("SPACE_SHOOT"));
		return true;
	}
	return handled;
}

void TgSpaceModel::applyWrongLetterPenalty()
{
	m_score -= 400;
}

const std::vector<TgSpaceEnemy *> &TgSpaceModel::enemies() const
{
	return m_enemies;
}

const std::vector<TgSpaceBullet *> &TgSpaceModel::bullets() const
{
	return m_bullets;
}

const std::vector<TgSpaceExplosion *> &TgSpaceModel::explosions() const
{
	return m_explosions;
}

QRectF TgSpaceModel::playerRect() const
{
	const qreal cy = m_viewH * 0.88;
	return QRectF(m_playerX - kPlayerHalfW, cy - kPlayerHalfH, kPlayerHalfW * 2.0, kPlayerHalfH * 2.0);
}

QRectF TgSpaceModel::enemyRect(const TgSpaceEnemy &enemy) const
{
	return QRectF(enemy.m_x - kEnemyHalf, enemy.m_y - kEnemyHalf, kEnemyHalf * 2.0, kEnemyHalf * 2.0);
}

qreal TgSpaceModel::scrollOffset() const
{
	return m_scroll;
}

QString TgSpaceModel::bonusWord() const
{
	return m_bonusWord;
}

QRectF TgSpaceModel::bonusWordRect() const
{
	const qreal w = 220.0;
	const qreal h = 36.0;
	return QRectF(m_bonusX, m_bonusY, w, h);
}

bool TgSpaceModel::hasBonusWord() const
{
	return m_bonusActive && !m_bonusWord.isEmpty();
}

int TgSpaceModel::bonusProgress() const
{
	return std::max(0, m_bonusProgress);
}

void TgSpaceModel::setBonusWord(const QString &word)
{
	m_bonusWord = word;
	m_bonusActive = !word.isEmpty();
	m_bonusProgress = 0;
	m_bonusX = -260.0;
	m_bonusY = m_viewH * 0.35;
	m_bonusVx = 180.0;
	if (!word.isEmpty())
		TgAudioManager::instance().playSpace(QStringLiteral("SPACE_WORDOUT"));
}

bool TgSpaceModel::tryProgressBonusWord(char letter)
{
	if (m_bonusWord.isEmpty())
		return false;
	if (m_bonusProgress >= m_bonusWord.size())
		return false;
	const char u = static_cast<char>(std::toupper(static_cast<unsigned char>(letter)));
	const QChar expected = m_bonusWord.at(m_bonusProgress).toUpper();
	if (expected != QChar::fromLatin1(u))
		return false;
	m_bonusProgress++;
	if (m_bonusProgress >= m_bonusWord.size())
	{
		m_hp = 18;
		m_bonusActive = false;
		m_bonusWord.clear();
		m_bonusProgress = 0;
	}
	return true;
}

bool TgSpaceModel::isEnemyLetterUsed(char letter) const
{
	for (const TgSpaceEnemy *p : m_enemies)
	{
		if (p->m_explosionFrame >= 0)
			continue;
		if (p->m_letter == letter)
			return true;
	}
	return false;
}

char TgSpaceModel::pickEnemyLetter() const
{
	for (int attempt = 0; attempt < 64; ++attempt)
	{
		const char c = static_cast<char>('A' + (qrand() % 26));
		if (!isEnemyLetterUsed(c))
			return c;
	}
	return 'A';
}

void TgSpaceModel::spawnEnemyIfNeeded(qreal deltaSec)
{
	if (static_cast<int>(m_enemies.size()) >= m_maxEnemies)
		return;
	m_spawnCooldown -= deltaSec;
	if (m_spawnCooldown > 0.0)
		return;
	m_spawnCooldown = frand(kSpawnIntervalMin, kSpawnIntervalMax);

	TgSpaceEnemy *p = m_enemyPool.acquire();
	if (nullptr == p)
		return;
	const bool useMeteor = m_maxEnemies >= 6 && (qrand() % 2 == 0);
	p->m_kind = useMeteor ? TgSpaceEnemyKind::MeteorEnemyKind : TgSpaceEnemyKind::PlaneEnemyKind;
	p->m_y = -kEnemyHalf * 2.0;
	p->m_x = frand(kEnemyHalf, static_cast<qreal>(m_viewW) - kEnemyHalf);
	p->m_phase = frand(0.0, 6.28);
	const qreal baseVy = 90.0 + static_cast<qreal>(m_speedLevel) * 22.0;
	p->m_vy = baseVy;
	p->m_vx = (p->m_kind == TgSpaceEnemyKind::PlaneEnemyKind) ? 60.0 : 0.0;
	p->m_letter = pickEnemyLetter();
	p->m_hittable = false;
	p->m_explosionFrame = -1;
	m_enemies.push_back(p);
	TgAudioManager::instance().playSpace(QStringLiteral("SPACE_PLANEOUT"));
}

void TgSpaceModel::pruneExplosions(qreal deltaSec)
{
	Q_UNUSED(deltaSec);
	for (auto it = m_explosions.begin(); it != m_explosions.end();)
	{
		TgSpaceExplosion *e = *it;
		e->m_frame++;
		if (e->m_frame > 12)
		{
			m_explosionPool.release(e);
			it = m_explosions.erase(it);
			continue;
		}
		++it;
	}
}

void TgSpaceModel::movePlayer(qreal deltaSec)
{
	const qreal speed = 220.0;
	m_playerX += m_playerDir * speed * deltaSec;
	const qreal margin = kPlayerHalfW + 8.0;
	if (m_playerX < margin)
	{
		m_playerX = margin;
		m_playerDir = 1.0;
	}
	else if (m_playerX > m_viewW - margin)
	{
		m_playerX = m_viewW - margin;
		m_playerDir = -1.0;
	}
}

void TgSpaceModel::moveBullets(qreal deltaSec)
{
	for (auto it = m_bullets.begin(); it != m_bullets.end();)
	{
		TgSpaceBullet *b = *it;
		if (nullptr == b->m_pTarget)
		{
			m_bulletPool.release(b);
			it = m_bullets.erase(it);
			continue;
		}
		if (b->m_pTarget->m_explosionFrame >= 0)
		{
			m_bulletPool.release(b);
			it = m_bullets.erase(it);
			continue;
		}
		const QRectF tr = enemyRect(*b->m_pTarget);
		const qreal tcx = tr.center().x();
		const qreal tcy = tr.center().y();
		qreal dx = tcx - b->m_x;
		qreal dy = tcy - b->m_y;
		const qreal len = std::sqrt(dx * dx + dy * dy);
		if (len < 8.0)
		{
			TgSpaceEnemy *t = b->m_pTarget;
			t->m_explosionFrame = 0;
			m_score += 1500;
			TgAudioManager::instance().playSpace(QStringLiteral("SPACE_BLAST"));
			TgSpaceExplosion *ex = m_explosionPool.acquire();
			if (nullptr != ex)
			{
				ex->m_x = t->m_x;
				ex->m_y = t->m_y;
				ex->m_frame = 0;
				m_explosions.push_back(ex);
			}
			m_bulletPool.release(b);
			it = m_bullets.erase(it);
			continue;
		}
		dx /= len;
		dy /= len;
		const qreal step = std::min(len, kBulletSpeed * deltaSec);
		b->m_x += dx * step;
		b->m_y += dy * step;
		++it;
	}
}

void TgSpaceModel::moveEnemies(qreal deltaSec)
{
	for (auto it = m_enemies.begin(); it != m_enemies.end();)
	{
		TgSpaceEnemy *p = *it;
		if (p->m_explosionFrame >= 0)
		{
			p->m_explosionFrame++;
			if (p->m_explosionFrame > 14)
			{
				releaseEnemy(p);
				it = m_enemies.erase(it);
				continue;
			}
			++it;
			continue;
		}
		p->m_phase += deltaSec * 2.2;
		if (p->m_kind == TgSpaceEnemyKind::PlaneEnemyKind)
			p->m_x += std::sin(p->m_phase) * 90.0 * deltaSec;
		p->m_y += p->m_vy * deltaSec;

		const QRectF er = enemyRect(*p);
		p->m_hittable = rectFullyInsideView(er, m_viewW, m_viewH);

		if (p->m_y - kEnemyHalf > m_viewH)
		{
			// Enemy leaving the screen should NOT damage player; only collision does.
			releaseEnemy(p);
			it = m_enemies.erase(it);
			continue;
		}

		if (er.intersects(playerRect()))
		{
			m_hp -= 1;
			// Collision should also trigger explosion + sound, same as being shot.
			p->m_explosionFrame = 0;
			TgAudioManager::instance().playSpace(QStringLiteral("SPACE_BLAST"));
			TgSpaceExplosion *ex = m_explosionPool.acquire();
			if (nullptr != ex)
			{
				ex->m_x = p->m_x;
				ex->m_y = p->m_y;
				ex->m_frame = 0;
				m_explosions.push_back(ex);
			}
			++it;
			continue;
		}
		++it;
	}
}

void TgSpaceModel::tickBonusWord(qreal deltaSec)
{
	if (!m_bonusActive || m_bonusWord.isEmpty())
		return;
	m_bonusX += m_bonusVx * deltaSec;
	if (m_bonusX > m_viewW + 300.0)
	{
		m_bonusActive = false;
		m_bonusWord.clear();
		m_bonusProgress = 0;
	}
}

void TgSpaceModel::tickDifficulty(qreal deltaSec)
{
	m_upgradeCountdown -= deltaSec;
	if (m_upgradeCountdown <= 0.0)
	{
		m_upgradeCountdown = static_cast<qreal>(m_upgradeIntervalSec);
		bool upgraded = false;
		if (m_speedLevel < 10)
		{
			m_speedLevel++;
			upgraded = true;
		}
		if (m_maxEnemies < 10)
		{
			m_maxEnemies++;
			upgraded = true;
		}
		if (upgraded)
			TgAudioManager::instance().playSpace(QStringLiteral("UPGRADE"));
	}
}

void TgSpaceModel::releaseEnemy(TgSpaceEnemy *pEnemy)
{
	for (auto it = m_bullets.begin(); it != m_bullets.end();)
	{
		TgSpaceBullet *b = *it;
		if (b->m_pTarget == pEnemy)
		{
			m_bulletPool.release(b);
			it = m_bullets.erase(it);
			continue;
		}
		++it;
	}
	m_enemyPool.release(pEnemy);
}

