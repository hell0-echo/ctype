#ifndef _TYPEGAME_TG_SPACE_MODEL_H_
#define _TYPEGAME_TG_SPACE_MODEL_H_

#include "common/tg_objectpool.h"

#include <QRectF>
#include <QString>
#include <vector>

enum class TgSpaceGameState
{
	InitialSpaceState,
	PlayingSpaceState,
	PausedSpaceState,
	EndSpaceState
};

enum class TgSpaceEnemyKind
{
	PlaneEnemyKind,
	MeteorEnemyKind
};

struct TgSpaceEnemy
{
	qreal m_x = 0.0;
	qreal m_y = 0.0;
	qreal m_vx = 0.0;
	qreal m_vy = 0.0;
	qreal m_phase = 0.0;
	char m_letter = 'A';
	TgSpaceEnemyKind m_kind = TgSpaceEnemyKind::PlaneEnemyKind;
	bool m_hittable = false;
	int m_explosionFrame = -1;
};

struct TgSpaceBullet
{
	qreal m_x = 0.0;
	qreal m_y = 0.0;
	TgSpaceEnemy *m_pTarget = nullptr;
};

struct TgSpaceExplosion
{
	qreal m_x = 0.0;
	qreal m_y = 0.0;
	int m_frame = 0;
};

class TgSpaceModel
{
public:
	TgSpaceModel();

	TgSpaceModel(const TgSpaceModel &) = delete;
	TgSpaceModel &operator=(const TgSpaceModel &) = delete;

	void setViewportSize(int width, int height);
	void setMaxEnemies(int value);
	void setSpeedLevel(int value);
	void setUpgradeIntervalSec(int value);
	void setBonusMode(bool on);

	int score() const;
	int hp() const;
	int maxEnemies() const;
	int speedLevel() const;
	int upgradeIntervalSec() const;
	bool bonusMode() const;
	qreal upgradeCountdownSec() const;

	TgSpaceGameState state() const;
	void setState(TgSpaceGameState state);

	void startFromMenu();
	void resumeFromPause();
	void pauseToMenu();
	void resetToInitial();

	void update(qreal deltaSec);

	bool tryFireAtLetter(char letter);
	void applyWrongLetterPenalty();

	const std::vector<TgSpaceEnemy *> &enemies() const;
	const std::vector<TgSpaceBullet *> &bullets() const;
	const std::vector<TgSpaceExplosion *> &explosions() const;

	QRectF playerRect() const;
	QRectF enemyRect(const TgSpaceEnemy &enemy) const;
	qreal scrollOffset() const;

	QString bonusWord() const;
	QRectF bonusWordRect() const;
	bool hasBonusWord() const;

	void setBonusWord(const QString &word);
	bool tryProgressBonusWord(char letter);

private:
	void spawnEnemyIfNeeded(qreal deltaSec);
	void pruneExplosions(qreal deltaSec);
	void movePlayer(qreal deltaSec);
	void moveBullets(qreal deltaSec);
	void moveEnemies(qreal deltaSec);
	void tickBonusWord(qreal deltaSec);
	void tickDifficulty(qreal deltaSec);
	void releaseEnemy(TgSpaceEnemy *pEnemy);

	int m_viewW = 800;
	int m_viewH = 600;

	int m_score = 0;
	int m_hp = 18;
	int m_maxEnemies = 3;
	int m_speedLevel = 3;
	int m_upgradeIntervalSec = 120;
	bool m_bonusMode = false;

	TgSpaceGameState m_state = TgSpaceGameState::InitialSpaceState;

	qreal m_playerX = 400.0;
	qreal m_playerDir = 1.0;
	qreal m_scroll = 0.0;

	qreal m_upgradeCountdown = 120.0;

	TgObjectPool<TgSpaceEnemy, 32> m_enemyPool;
	TgObjectPool<TgSpaceBullet, 32> m_bulletPool;
	TgObjectPool<TgSpaceExplosion, 64> m_explosionPool;

	std::vector<TgSpaceEnemy *> m_enemies;
	std::vector<TgSpaceBullet *> m_bullets;
	std::vector<TgSpaceExplosion *> m_explosions;

	QString m_bonusWord;
	int m_bonusProgress = 0;
	qreal m_bonusX = 0.0;
	qreal m_bonusY = 0.0;
	qreal m_bonusVx = 120.0;
	bool m_bonusActive = false;

	qreal m_spawnCooldown = 0.0;

	bool isEnemyLetterUsed(char letter) const;
	char pickEnemyLetter() const;
};

#endif
