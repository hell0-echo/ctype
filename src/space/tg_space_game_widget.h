#ifndef _TYPEGAME_TG_SPACE_GAME_WIDGET_H_
#define _TYPEGAME_TG_SPACE_GAME_WIDGET_H_

#include <QWidget>

class QPushButton;
class TgSpaceController;
class TgSpaceModel;
class TgSpaceView;
class TgConfig;

class TgSpaceGameWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TgSpaceGameWidget(TgSpaceModel* pModel, TgSpaceController* pController,
        TgSpaceView* pView, TgConfig* pConfig, QWidget* pParent = nullptr);
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

signals:
    void highScoreRequested();
    void exitToLauncherRequested();

private:
    void updateButtonStates();

    TgSpaceModel* m_pModel = nullptr;
    TgSpaceController* m_pController = nullptr;
    TgSpaceView* m_pView = nullptr;
    TgConfig* m_pConfig = nullptr;

    QPushButton* m_pPauseBtn = nullptr;
    QPushButton* m_pStartBtn = nullptr;
    QPushButton* m_pHighScoreBtn = nullptr;
    QPushButton* m_pSettingsBtn = nullptr;
    QPushButton* m_pExitBtn = nullptr;
};
#endif