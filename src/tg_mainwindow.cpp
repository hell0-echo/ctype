#include "tg_mainwindow.h"
#include "apple/tg_apple_controller.h"
#include "apple/tg_apple_game_widget.h"
#include "apple/tg_apple_model.h"
#include "apple/tg_apple_view.h"
#include "space/tg_space_controller.h"
#include "space/tg_space_game_widget.h"
#include "space/tg_space_model.h"
#include "space/tg_space_view.h"
#include "ui/tg_bonuswordprovider.h"
#include "ui/tg_dialogs.h"
#include "common/tg_audiomanager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QStackedWidget>
#include <QVBoxLayout>
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
    auto* pW = new QWidget;
    auto* pLay = new QVBoxLayout(pW);

    auto* pApple = new QPushButton(QStringLiteral("Apple rescue"));
    auto* pSpace = new QPushButton(QStringLiteral("Space battle"));
    auto* pExit = new QPushButton(QStringLiteral("Exit"));

    pLay->addStretch(1);
    pLay->addWidget(pApple);
    pLay->addWidget(pSpace);
    pLay->addWidget(pExit);
    pLay->addStretch(1);

    connect(pApple, &QPushButton::clicked, this, &TgMainWindow::onShowApple);
    connect(pSpace, &QPushButton::clicked, this, &TgMainWindow::onShowSpace);
    connect(pExit, &QPushButton::clicked, this, &TgMainWindow::close);

    m_pStack->addWidget(pW);
}

void TgMainWindow::buildApplePage()
{
    m_pAppleModel = new TgAppleModel();
    m_pAppleController = new TgAppleController(m_pAppleModel, this);
    m_pAppleView = new TgAppleView(m_pAppleModel, &m_resources, this);
    m_pApplePage = new TgAppleGameWidget(m_pAppleModel, m_pAppleController, m_pAppleView, &m_config, this);

    connect(m_pAppleController, &TgAppleController::modelUpdated, this, &TgMainWindow::onAppleModelUpdated);
    connect(m_pAppleController, &TgAppleController::roundEnded, this, &TgMainWindow::onAppleRoundEnded);

    m_pStack->addWidget(m_pApplePage);
}

void TgMainWindow::buildSpacePage()
{
    m_pSpaceModel = new TgSpaceModel();
    m_pBonusProvider = new TgBonusWordProvider(this);
    m_pBonusProvider->setEndpoint(m_config.spaceBonusApiEndpoint());
    m_pBonusProvider->setApiKey(m_config.spaceBonusApiKey());

    auto* pController = new TgSpaceController(m_pSpaceModel, m_pBonusProvider, this);
    auto* pView = new TgSpaceView(m_pSpaceModel, &m_resources, this);

    m_pSpaceGameWidget = new TgSpaceGameWidget(m_pSpaceModel, pController, pView, &m_config, &m_scores, &m_resources, this);

    connect(pController, &TgSpaceController::modelUpdated, this, &TgMainWindow::onSpaceModelUpdated);
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
    if (isWin) {
        const int r = QMessageBox::question(this, QString(),
            QStringLiteral("Stage cleared. Increase level for next run?"),
            QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes) m_pAppleModel->setLevel(std::min(9, m_pAppleModel->level() + 1));
        m_pApplePage->onStartClicked();
    }
    else {
        QMessageBox::information(this, QString(), QStringLiteral("Try again."));
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
    const QString name = QInputDialog::getText(this, QStringLiteral("High score"), QStringLiteral("Name:"));
    m_scores.submitScore(name, m_pSpaceModel->score());
    m_scores.save();
    QMessageBox::information(this, QString(), QStringLiteral("Game over"));
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