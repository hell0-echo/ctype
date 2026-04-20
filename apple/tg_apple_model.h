#ifndef _TYPEGAME_TG_APPLE_MODEL_H_
#define _TYPEGAME_TG_APPLE_MODEL_H_

#include "common/tg_objectpool.h"

#include <QRectF>
#include <QString>
#include <vector>

enum class TgAppleGameState
{
	IdleAppleState,
	PlayingAppleState,
	PausedAppleState,
	EndAppleState
};

enum class TgAppleEntityState
{
	NormalAppleEntity,
	BrokenAppleEntity
};

struct TgAppleEntity
{
	qreal m_x = 0.0;
	qreal m_y = 0.0;
	qreal m_speedY = 0.0;
	qreal m_brokenAge = 0.0;
	char m_letter = 'A';
	TgAppleEntityState m_entityState = TgAppleEntityState::NormalAppleEntity;
};

class TgAppleModel
{
public:
	TgAppleModel();

	TgAppleModel(const TgAppleModel &) = delete;
	TgAppleModel &operator=(const TgAppleModel &) = delete;

	void setViewportSize(int width, int height);
	void setLevel(int level);
	void setFailLimit(int value);
	void setWinTarget(int value);
	void setMaxApplesOnScreen(int value);

	int level() const;
	int score() const;
	int mistakes() const;
	int winTarget() const;
	int failLimit() const;
	int maxApplesOnScreen() const;

	TgAppleGameState state() const;
	void setState(TgAppleGameState state);

	void startNewRound();
	void resetIdle();

	void update(qreal deltaSec);

	bool tryHitLetter(char letter);

	qreal failLineY() const;
	QRectF appleRect(const TgAppleEntity &entity) const;
	int basketAppleCount() const;

	const std::vector<TgAppleEntity *> &activeApples() const;

private:
	void spawnAppleIfNeeded();
	bool isLetterUsed(char letter) const;
	char pickUnusedLetter() const;
	void releaseEntity(TgAppleEntity *pEntity);
	void updateBasketVisual();

	int m_viewW = 800;
	int m_viewH = 600;

	int m_level = 3;
	int m_score = 0;
	int m_mistakes = 0;
	int m_winTarget = 100;
	int m_failLimit = 10;
	int m_maxOnScreen = 5;

	TgAppleGameState m_state = TgAppleGameState::IdleAppleState;

	TgObjectPool<TgAppleEntity, 16> m_pool;
	std::vector<TgAppleEntity *> m_active;

	qreal m_spawnCooldown = 0.0;
	int m_basketApples = 0;
};

#endif
