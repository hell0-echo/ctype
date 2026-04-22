#ifndef _TYPEGAME_TG_APPLE_CONTROLLER_H_
#define _TYPEGAME_TG_APPLE_CONTROLLER_H_

#include "tg_apple_model.h"

#include <QElapsedTimer>
#include <QObject>

class QTimer;
class TgAppleModel;

class TgAppleController : public QObject
{
	Q_OBJECT

public:
	explicit TgAppleController(TgAppleModel *pModel, QObject *pParent = nullptr);

	TgAppleController(const TgAppleController &) = delete;
	TgAppleController &operator=(const TgAppleController &) = delete;

	void startLoop();
	void stopLoop();

	void setPaused(bool paused);

signals:
	void modelUpdated();
	void roundEnded(bool isWin);

private slots:
	void onTimerTick();

private:
	TgAppleModel *m_pModel = nullptr;
	QTimer *m_pTimer = nullptr;
	QElapsedTimer m_elapsed;
	bool m_paused = false;
	TgAppleGameState m_lastState = TgAppleGameState::IdleAppleState;
};

#endif
