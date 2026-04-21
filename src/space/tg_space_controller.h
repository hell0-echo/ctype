#ifndef _TYPEGAME_TG_SPACE_CONTROLLER_H_
#define _TYPEGAME_TG_SPACE_CONTROLLER_H_

#include "tg_space_model.h"

#include <QElapsedTimer>
#include <QObject>

class QTimer;
class TgBonusWordProvider;

class TgSpaceController : public QObject
{
	Q_OBJECT

public:
	explicit TgSpaceController(TgSpaceModel *pModel, TgBonusWordProvider *pBonus, QObject *pParent = nullptr);

	TgSpaceController(const TgSpaceController &) = delete;
	TgSpaceController &operator=(const TgSpaceController &) = delete;

	void startLoop();
	void stopLoop();
	void setPaused(bool paused);
	void setBonusModeEnabled(bool on);

signals:
	void modelUpdated();
	void roundEnded();

private slots:
	void onTick();
	void onBonusTimer();
	void onBonusWord(const QString &word);

private:
	TgSpaceModel *m_pModel = nullptr;
	TgBonusWordProvider *m_pBonus = nullptr;
	QTimer *m_pTimer = nullptr;
	QTimer *m_pBonusTimer = nullptr;
	QElapsedTimer m_elapsed;
	bool m_paused = false;
	bool m_bonusEnabled = false;
	TgSpaceGameState m_lastState = TgSpaceGameState::InitialSpaceState;
};

#endif
