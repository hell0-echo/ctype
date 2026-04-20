#ifndef _TYPEGAME_TG_MAINWINDOW_H_
#define _TYPEGAME_TG_MAINWINDOW_H_

#include "common/tg_config.h"
#include "common/tg_resourceprovider.h"
#include "common/tg_scorestore.h"

#include <QCloseEvent>
#include <QMainWindow>
#include <QString>

class QStackedWidget;
class TgAppleGameWidget;
class TgAppleModel;
class TgAppleController;
class TgAppleView;
class TgSpaceModel;
class TgSpaceController;
class TgSpaceView;
class TgBonusWordProvider;

class TgMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit TgMainWindow(QWidget *pParent = nullptr);

	TgMainWindow(const TgMainWindow &) = delete;
	TgMainWindow &operator=(const TgMainWindow &) = delete;

protected:
	void closeEvent(QCloseEvent *pEvent) override;

private slots:
	void onShowHome();
	void onShowApple();
	void onShowSpace();

	void onAppleModelUpdated();
	void onAppleRoundEnded(bool isWin);

	void onSpaceModelUpdated();
	void onSpaceRoundEnded();

	void onSpaceStart();
	void onSpaceContinue();
	void onSpacePause();
	void onSpaceHighScore();
	void onSpaceSettings();
	void onSpaceExitMenu();

private:
	void buildHome();
	void buildApplePage();
	void buildSpacePage();
	bool runSpaceSettings();

	TgConfig m_config;
	TgResourceProvider m_resources;
	TgScoreStore m_scores;

	QStackedWidget *m_pStack = nullptr;

	TgAppleModel *m_pAppleModel = nullptr;
	TgAppleController *m_pAppleController = nullptr;
	TgAppleView *m_pAppleView = nullptr;
	TgAppleGameWidget *m_pApplePage = nullptr;

	TgSpaceModel *m_pSpaceModel = nullptr;
	TgSpaceController *m_pSpaceController = nullptr;
	TgSpaceView *m_pSpaceView = nullptr;
	TgBonusWordProvider *m_pBonusProvider = nullptr;
};

#endif
