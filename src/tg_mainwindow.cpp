#include "tg_mainwindow.h"
#include "apple/tg_apple_controller.h"
#include "apple/tg_apple_game_widget.h"
#include "apple/tg_apple_model.h"
#include "apple/tg_apple_result_dialog.h"
#include "apple/tg_apple_view.h"
#include "space/tg_space_controller.h"
#include "space/tg_space_game_widget.h"
#include "space/tg_space_model.h"
#include "space/tg_space_result_dialog.h"
#include "space/tg_space_view.h"
#include "ui/tg_bonuswordprovider.h"
#include "ui/tg_dialogs.h"
#include "ui/tg_home_widget.h"
#include "common/tg_audiomanager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QStackedWidget>
#include <algorithm>

TgMainWindow::TgMainWindow(QWidget* pParent)
    : QMainWindow(pParent)
    , m_config(QStringLiteral("typegame"), QStringLiteral("typegame"))
    , m_scores(QCoreApplication::applicationDirPath() + QStringLiteral("/scores.ini"))
{
    m_config.load();
    m_scores.load();
    TgAudioManager::instance().preload();

    m_pStack = new QStackedWidget(this);
    setCentralWidget(m_pStack);

    buildHome();
    buildApplePage();
    buildSpacePage();

    onShowHome();
}

void TgMainWindow::buildHome()
{
    auto* pHome = new TgHomeWidget;
    connect(pHome, &TgHomeWidget::appleRequested, this, &TgMainWindow::onShowApple);
    connect(pHome, &TgHomeWidget::spaceRequested, this, &TgMainWindow::onShowSpace);
    m_pStack->addWidget(pHome);
}

void TgMainWindow::buildApplePage()
{
    m_pAppleModel = new TgAppleModel();
    m_pAppleController = new TgAppleController(m_pAppleModel, this);
    m_pAppleView = new TgAppleView(m_pAppleModel, &m_resources, this);
    m_pApplePage = new TgAppleGameWidget(m_pAppleModel, m_pAppleController, m_pAppleView, &m_config, this);

    connect(m_pAppleController, &TgAppleController::modelUpdated, this, &TgMainWindow::onAppleModelUpdated);
    connect(m_pAppleController, &TgAppleController::roundEnded, this, &TgMainWindow::onAppleRoundEnded);
    connect(m_pApplePage, &TgAppleGameWidget::exitToLauncherRequested, this, &TgMainWindow::onShowHome);

    m_pStack->addWidget(m_pApplePage);
}

void TgMainWindow::buildSpacePage()
{
    m_pSpaceModel = new TgSpaceModel();
    m_pBonusProvider = new TgBonusWordProvider(this);
    m_pBonusProvider->setApiKey(m_config.spaceBonusApiKey());

    auto* pController = new TgSpaceController(m_pSpaceModel, m_pBonusProvider, this);
    auto* pView = new TgSpaceView(m_pSpaceModel, &m_resources, this);

    m_pSpaceGameWidget = new TgSpaceGameWidget(m_pSpaceModel, pController, pView, &m_config, &m_scores, &m_resources, m_pBonusProvider, this);

    connect(pController, &TgSpaceController::modelUpdated, this, &TgMainWindow::onSpaceModelUpdated);
    connect(pController, &TgSpaceController::modelUpdated, pView, &TgSpaceView::onModelUpdated);
    connect(pController, &TgSpaceController::modelUpdated, pView, QOverload<>::of(&QWidget::update));
    connect(pController, &TgSpaceController::roundEnded, this, &TgMainWindow::onSpaceRoundEnded);
    connect(pController, &TgSpaceController::roundEnded, m_pSpaceGameWidget, &TgSpaceGameWidget::onRoundEnded);

    connect(m_pSpaceGameWidget, &TgSpaceGameWidget::highScoreRequested, this, &TgMainWindow::onSpaceHighScore);
    connect(m_pSpaceGameWidget, &TgSpaceGameWidget::exitToLauncherRequested, this, &TgMainWindow::onShowHome);

    m_pStack->addWidget(m_pSpaceGameWidget);
}

void TgMainWindow::closeEvent(QCloseEvent* pEvent)
{
    m_config.save();
    QMainWindow::closeEvent(pEvent);
}

void TgMainWindow::onShowHome()
{
    TgAudioManager::instance().stopBgm();
    m_pStack->setCurrentIndex(0);
}

void TgMainWindow::onShowApple()
{
    m_pAppleView->refreshMetrics();
    m_pStack->setCurrentIndex(1);
    m_pApplePage->syncUiState();
}

void TgMainWindow::onShowSpace()
{
    m_pSpaceGameWidget->refreshLayout();
    m_pSpaceGameWidget->setFocus();
    m_pStack->setCurrentIndex(2);
}

void TgMainWindow::onAppleModelUpdated()
{
    m_pAppleView->update();
}

void TgMainWindow::onAppleRoundEnded(bool isWin)
{
    m_pApplePage->syncUiState();
    TgAudioManager::instance().stopBgm();
    TgAppleResultDialog dlg(&m_resources, isWin, this);
    dlg.exec();
    const TgAppleResultDialog::Action a = dlg.action();
    if (a == TgAppleResultDialog::Action::Replay)
    {
        m_pApplePage->onStartClicked();
        return;
    }
    if (a == TgAppleResultDialog::Action::Next)
    {
        // Next level should take effect immediately. `onStartClicked()` reloads level from config,
        // so we must update config first (model-only change would be overwritten).
        m_config.setAppleLevel(std::min(9, m_config.appleLevel() + 1));
        m_pApplePage->onStartClicked();
        return;
    }
    if (a == TgAppleResultDialog::Action::End)
    {
        m_pApplePage->onEndClicked();
        return;
    }
}

void TgMainWindow::onSpaceModelUpdated()
{
    // 渲染更新已由 Controller 内部或 View 的 update() 处理，此处无需额外逻辑
}

void TgMainWindow::onSpaceRoundEnded()
{
    if (!m_pSpaceModel) return;
    TgAudioManager::instance().stopBgm();
    QInputDialog nameDlg(this);
    nameDlg.setWindowTitle(QString::fromUtf16(u"\u9ad8\u5206\u82f1\u96c4"));
    nameDlg.setLabelText(QString::fromUtf16(u"\u8bf7\u7559\u4e0b\u60a8\u7684\u5927\u540d"));
    nameDlg.setTextValue(QString::fromUtf16(u"\u82f1\u96c4"));
    nameDlg.setOkButtonText(QStringLiteral("OK"));
    nameDlg.setCancelButtonText(QStringLiteral("Cancel"));
    nameDlg.setWindowFlags((nameDlg.windowFlags() & ~Qt::WindowContextHelpButtonHint) | Qt::Dialog);

    QString name = QString::fromUtf16(u"\u82f1\u96c4");
    const bool accepted = (nameDlg.exec() == QDialog::Accepted);
    if (accepted)
    {
        name = nameDlg.textValue().trimmed();
        m_scores.submitScore(name, m_pSpaceModel->score());
        m_scores.save();
    }

    // Either OK (saved) or Cancel (not saved): return to Space main menu page.
    if (m_pSpaceGameWidget)
        m_pSpaceGameWidget->showMainMenu();
}

void TgMainWindow::onSpaceHighScore()
{
    QString text;
    const auto list = m_scores.topTen();
    int rank = 1;
    for (const TgScoreEntry& e : list) {
        text += QStringLiteral("%1. %2  %3\n").arg(rank).arg(e.m_name).arg(e.m_score);
        rank++;
    }
    if (text.isEmpty()) text = QStringLiteral("(empty)");
    QMessageBox::information(this, QStringLiteral("High scores"), text);
}