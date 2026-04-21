#ifndef _TYPEGAME_TG_MAINWINDOW_H_
#define _TYPEGAME_TG_MAINWINDOW_H_

#include "common/tg_config.h"
#include "common/tg_resourceprovider.h"
#include "common/tg_scorestore.h"
#include "space/tg_space_game_widget.h" // 新增
#include <QCloseEvent>
#include <QMainWindow>
#include <QString>

class QStackedWidget;
class TgAppleGameWidget;
class TgAppleModel;
class TgAppleController;
class TgAppleView;
class TgSpaceModel; // 保留仅用于分数读取
class TgBonusWordProvider; // 保留仅用于构造传递

class TgMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit TgMainWindow(QWidget* pParent = nullptr);
    TgMainWindow(const TgMainWindow&) = delete;
    TgMainWindow& operator=(const TgMainWindow&) = delete;

protected:
    void closeEvent(QCloseEvent* pEvent) override;

private slots:
    void onShowHome();
    void onShowApple();
    void onShowSpace();
    void onAppleModelUpdated();
    void onAppleRoundEnded(bool isWin);
    void onSpaceModelUpdated();
    void onSpaceRoundEnded(); // 仅处理分数持久化
    void onSpaceHighScore();

private:
    void buildHome();
    void buildApplePage();
    void buildSpacePage();

    TgConfig m_config;
    TgResourceProvider m_resources;
    TgScoreStore m_scores;
    QStackedWidget* m_pStack = nullptr;

    TgAppleModel* m_pAppleModel = nullptr;
    TgAppleController* m_pAppleController = nullptr;
    TgAppleView* m_pAppleView = nullptr;
    TgAppleGameWidget* m_pApplePage = nullptr;

    TgSpaceModel* m_pSpaceModel = nullptr; // 仅保留非 owning 指针用于读取 score()
    TgSpaceGameWidget* m_pSpaceGameWidget = nullptr;
    TgBonusWordProvider* m_pBonusProvider = nullptr;
};
#endif