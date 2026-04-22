#ifndef _TYPEGAME_TG_SPACE_GAME_WIDGET_H_
#define _TYPEGAME_TG_SPACE_GAME_WIDGET_H_

#include <QWidget>

class TgSpaceController;
class TgSpaceModel;
class TgSpaceView;
class TgConfig;
class TgScoreStore;
class TgResourceProvider;
class QStackedWidget;
class TgSpaceMenuWidget;
class TgSpaceRankWidget;

class TgSpaceGameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TgSpaceGameWidget(TgSpaceModel* pModel, TgSpaceController* pController,
        TgSpaceView* pView, TgConfig* pConfig, TgScoreStore* pScores, TgResourceProvider* pResources, QWidget* pParent = nullptr);
    TgSpaceGameWidget(const TgSpaceGameWidget&) = delete;
    TgSpaceGameWidget& operator=(const TgSpaceGameWidget&) = delete;

    void syncUiState();
    void refreshLayout();

public slots:
    void onPauseClicked();
    void onStartClicked();
    void onHighScoreClicked();
    void onSettingsClicked();
    void onExitClicked();
    void onRoundEnded(); // 新增：处理回合结束后的状态复位
    void onEscapeMenuRequested();
    void onMenuStart();
    void onMenuRank();
    void onMenuOption();
    void onMenuExit();
    void onRankReturn();

signals:
    void highScoreRequested();
    void exitToLauncherRequested();

private:
    void updateButtonStates();
    void showMenuPage();
    void showGamePage();
    void showRankPage();

    TgSpaceModel* m_pModel = nullptr;
    TgSpaceController* m_pController = nullptr;
    TgSpaceView* m_pView = nullptr;
    TgConfig* m_pConfig = nullptr;
    TgScoreStore* m_pScores = nullptr;
    TgResourceProvider* m_pResources = nullptr;

    QStackedWidget* m_pStack = nullptr;
    TgSpaceMenuWidget* m_pMenu = nullptr;
    TgSpaceRankWidget* m_pRank = nullptr;
};
#endif